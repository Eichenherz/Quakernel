#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define LIMINE_API_REVISION 3
#include <limine.h>

#include <c_macros.h>
#include <c_lib.h>

#include "memory/memory.h"
#include "arch/interrupts.h"
#include "device/dtb_parse.h"
#include "font/font.h"


// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.


// Set the base revision to 3, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.
__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION( LIMINE_API_REVISION );

__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = LIMINE_API_REVISION
};

__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = LIMINE_API_REVISION
};

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = LIMINE_API_REVISION
};

__attribute__((used, section(".requests")))
static volatile struct limine_dtb_request dtb_request = {
    .id = LIMINE_DTB_REQUEST,
    .revision = LIMINE_API_REVISION
};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.
__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;



typedef struct framebuffer_t 
{
    void* address;
    uint64_t width;
    uint64_t height;
    uint64_t bytesPerRow;
    uint16_t bitsPerPixel;
    uint8_t memory_model;
    uint8_t red_mask_size;
    uint8_t red_mask_shift;
    uint8_t green_mask_size;
    uint8_t green_mask_shift;
    uint8_t blue_mask_size;
    uint8_t blue_mask_shift;
} framebuffer_t;

static framebuffer_t gFbo;


framebuffer_t InitFramebuffer()
{
    struct limine_framebuffer* limineFbo = framebuffer_request.response->framebuffers[0];
    framebuffer_t fbo = {
        .address = limineFbo->address,
        .width = limineFbo->width,
        .height = limineFbo->height,
        .bytesPerRow = limineFbo->pitch,
        .bitsPerPixel = limineFbo->bpp,
        .memory_model = limineFbo->memory_model,
        .red_mask_size = limineFbo->red_mask_size,
        .red_mask_shift = limineFbo->red_mask_shift,
        .green_mask_size = limineFbo->green_mask_size,
        .green_mask_shift = limineFbo->green_mask_shift,
        .blue_mask_size = limineFbo->blue_mask_size,
        .blue_mask_shift = limineFbo->blue_mask_shift,
    };
    return fbo;
}

uint64_t FramebufferGetSizeInBytes( const framebuffer_t* const pFb )
{
    return pFb->height * pFb->bytesPerRow;
}

inline uint32_t PackPixel(
    uint8_t r, uint8_t g, uint8_t b, 
    uint8_t rMaskShift, uint8_t gMaskShift, uint8_t bMaskShift,  
    uint8_t rMaskSize, uint8_t gMaskSize, uint8_t bMaskSize
) {
    uint32_t packed = 
    ( ( r >> (8 - rMaskSize) ) << rMaskShift )
    | ( ( g >> (8 - gMaskSize) ) << gMaskShift )
    | ( ( b >> (8 - bMaskSize) ) << bMaskShift );
    return packed;
}

// Note: we assume the framebuffer model is RGB with 32-bit pixels.
void FramebufferPutPixel( framebuffer_t* pFbo, uint16_t x, uint16_t y, uint32_t rgb )
{
    if( ( x >= pFbo->width ) || ( y >= pFbo->height ) )
    {
        // TODO: rezise blah blah
        return;
    }
    uint64_t bytesPerPixel = pFbo->bitsPerPixel / 8;
    // NOTE: avoid division
    uint64_t index = y * pFbo->bytesPerRow + x * bytesPerPixel;
    volatile uint32_t* pFb = pFbo->address;
    pFb[index] = rgb;
}

// TODO: add backbuffer ( double ) default to actual fb, present == memcpy
typedef struct fb_console_t 
{
    const font_desc* font;
    uint16_t cursorX;
    uint16_t cursorY;
    uint16_t width;
    uint16_t height;
} fb_console;

static fb_console gFbConsole;

fb_console DefaultFbConsole()
{
    fb_console fbConsole = {
        .font = &defaultMonospace, 
        .cursorX = 0, 
        .cursorY = 0, 
        .width = gFbo.width, 
        .height = gFbo.height
    };
    return fbConsole;
}

// TODO: add backend to direct stream, hook multiple straems
void QPrint( const char* string, uint32_t rgb )
{
    for( ; *string != 0; string++ ) 
    {
        const char currentChar = *string;
        
        if( ( '\n' == currentChar ) || ( gFbConsole.cursorX + gFbConsole.font->width >= gFbConsole.width ) )
        {
            gFbConsole.cursorX = 0;
            gFbConsole.cursorY += gFbConsole.font->height;
        }
        if( gFbConsole.cursorY + gFbConsole.font->height >= gFbConsole.height )
        {
            gFbConsole.cursorY = 0;
            //scroll
        }

        for( size_t y = 0; y < gFbConsole.font->height; y++ )
        {
            // NOTE: our font table is blocks of width by height in the ASCII order
            // NOTE: we assume the biggest glyph width is u64
            const uint64_t currentGlyphRow = gFbConsole.font->data[currentChar * gFbConsole.font->height + y];
            const uint64_t highestGlyphBitMask = 1ull << ( gFbConsole.font->width-1 );
            for( size_t x = 0; x < gFbConsole.font->width; x++ )
            {
                uint64_t currentGlyphX = currentGlyphRow;
                if( currentGlyphX & highestGlyphBitMask )
                {
                    FramebufferPutPixel( &gFbo, gFbConsole.cursorX + x, gFbConsole.cursorY + y, rgb );
                }
                currentGlyphX <<= 1;
            }
        }
        
        gFbConsole.cursorX += gFbConsole.font->width;
    }
}

static mem_linear_arena_t preInitArena;

// NOTE: this must match the name in the linker script.
void KernelMain() 
{
    // Ensure the bootloader actually understands our base revision (see spec).
    QK_CHECK( !LIMINE_BASE_REVISION_SUPPORTED );

    QK_CHECK( !hhdm_request.response );

    // NOTE: limine mem blocks with USABLE are guaranteed to be 4Kib alligned.
    QK_CHECK( !memmap_request.response || memmap_request.response->entry_count < 1 );
    for( uint64_t mmEntry = 0; mmEntry < memmap_request.response->entry_count; ++mmEntry )
    {
        const struct limine_memmap_entry* const refThisMMEntry = 
            memmap_request.response->entries[mmEntry];
        if( refThisMMEntry->type == LIMINE_MEMMAP_USABLE && refThisMMEntry->length >= 8 * KiB )
        {   
            mem_block_t memBlock = { 
                .baseAddr = refThisMMEntry->base + hhdm_request.response->offset, 
                .size = refThisMMEntry->length 
            };
            preInitArena = MemLinearArenaCreate( memBlock );
            break;
        }
    }


    for( uint64_t mmEntry = 0; mmEntry < memmap_request.response->entry_count; ++mmEntry )
    {
        const struct limine_memmap_entry* const refThisMMEntry = 
            memmap_request.response->entries[mmEntry];
        if( refThisMMEntry->type == LIMINE_MEMMAP_USABLE && refThisMMEntry->length >= 8 * KiB )
        {   
            mem_block_t memBlock = { 
                .baseAddr = refThisMMEntry->base + hhdm_request.response->offset, 
                .size = refThisMMEntry->length 
            };
        }
    }

    QK_CHECK( !framebuffer_request.response || framebuffer_request.response->framebuffer_count < 1 );
    QK_CHECK( !dtb_request.response || dtb_request.response->dtb_ptr == NULL );



    gFbo = InitFramebuffer();

    const void* pDtb = dtb_request.response->dtb_ptr;
    ValidateDtb( pDtb );

    gFbConsole = DefaultFbConsole();

    QPrint( "Hello Quakernel !", 0xff5555 );

    // We're done, just hang...
    hcf();
}
