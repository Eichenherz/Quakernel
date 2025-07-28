#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define LIMINE_API_REVISION 3
#include <limine.h>

#include <dtc/libfdt/libfdt.h>

#include "device/uart.h"
#include "arch/serial.h"
#include "font/font.h"

// Utils ?
#define HIGHEST_BIT_MSK(T) ~( ~0ull >> 1 )


static void PrintStrSerial( const char* str )
{
    while( *str ) {
        PutCharSerial( *(str++) );
    }
}

// Set the base revision to 3, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.
__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(3);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests")))
static volatile struct limine_dtb_request dtb_request = {
    .id = LIMINE_DTB_REQUEST,
    .revision = 0
};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.
__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

// Halt and catch fire function.
static void hcf() {
    asm ("cli");
    for (;;) {
        asm ("hlt");
    }
}

typedef struct limine_requests_t {
    struct limine_framebuffer *framebuffer;
    const void* pDtb;
} limine_requests;

static limine_requests gLimineRequests;

void InitLimineRequests()
{
    // Ensure the bootloader actually understands our base revision (see spec).
    if( !LIMINE_BASE_REVISION_SUPPORTED )
    {
        hcf();
    }

    // Ensure we got a framebuffer.
    if( !framebuffer_request.response || framebuffer_request.response->framebuffer_count < 1 )
    {
        hcf();
    }
    if( !dtb_request.response ) 
    {
        hcf();
    }
    // Fetch the first framebuffer.
    limine_requests req = { 
        .framebuffer = framebuffer_request.response->framebuffers[0], 
        .pDtb = dtb_request.response->dtb_ptr 
    };
    gLimineRequests = req;
}

typedef struct framebuffer_t {
    void* address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
    uint16_t bpp;
    uint8_t memory_model;
    uint8_t red_mask_size;
    uint8_t red_mask_shift;
    uint8_t green_mask_size;
    uint8_t green_mask_shift;
    uint8_t blue_mask_size;
    uint8_t blue_mask_shift;
} framebuffer;

static framebuffer gFbo;


void InitFramebuffer()
{
    struct limine_framebuffer* limineFbo = gLimineRequests.framebuffer;
    framebuffer fbo = {
        .address = limineFbo->address,
        .width = limineFbo->width,
        .height = limineFbo->height,
        .pitch = limineFbo->pitch,
        .bpp = limineFbo->bpp,
        .memory_model = limineFbo->memory_model,
        .red_mask_size = limineFbo->red_mask_size,
        .red_mask_shift = limineFbo->red_mask_shift,
        .green_mask_size = limineFbo->green_mask_size,
        .green_mask_shift = limineFbo->green_mask_shift,
        .blue_mask_size = limineFbo->blue_mask_size,
        .blue_mask_shift = limineFbo->blue_mask_shift,
    };
    gFbo = fbo;
}

// Note: we assume the framebuffer model is RGB with 32-bit pixels.
void FramebufferPutPixel( uint16_t x, uint16_t y, uint32_t rgb )
{
    if( ( x >= gFbo.width ) || ( y >= gFbo.height ) )
    {
        // TODO: panic ?
    }
    const uint16_t fboWidth = gFbo.pitch / ( gFbo.bpp / 8 );
    volatile uint32_t *fb_ptr = gFbo.address;
    fb_ptr[y * fboWidth + x] = rgb;
}

typedef struct fb_console_t {
    const font_desc* font;
    uint16_t cursorX;
    uint16_t cursorY;
    uint16_t width;
    uint16_t height;
} fb_console;

static fb_console gFbConsole;

void InitFbConsole()
{
    fb_console fbConsole = {
        .font = &defaultMonospace, 
        .cursorX = 0, 
        .cursorY = 0, 
        .width = gFbo.width, 
        .height = gFbo.height
    };
    gFbConsole = fbConsole;
    Com1Puts( "\n" );
}

void QPrint( const char* string, uint32_t rgb )
{
    PrintStrSerial( string );
    while( *string != 0 ) 
    {
        if( gFbConsole.cursorX + gFbConsole.font->width >= gFbConsole.width )
        {
            gFbConsole.cursorX = 0;
            gFbConsole.cursorY += gFbConsole.font->height;
        }

        for( size_t y = 0; y < gFbConsole.font->height; y++ )
        {
            char start = gFbConsole.font->data[*string * gFbConsole.font->height + y];
            for( size_t x = 0; x < gFbConsole.font->width; x++ )
            {
                if( start & HIGHEST_BIT_MSK(char) ) // Highest bit
                {
                    FramebufferPutPixel( gFbConsole.cursorX + x, gFbConsole.cursorY + y, rgb ); // WRONG
                }
                start <<= 1;
            }
        }
        
        string++;
        gFbConsole.cursorX += gFbConsole.font->width;
    }
}

const char DEVICE_DTB_OKAY[] = "okay";
const char DEVICE_DTB_OK[] = "ok";

const char UART_COMPAT[] = "ns16550a";
const char UART_KY_X1_COMPAT[] = "ky,pxa-uart";

const char* UART_ACCEPTED[] = { UART_COMPAT, UART_KY_X1_COMPAT };

void ParseDtb( const void* pDtb )
{
    uint32_t totalSz = fdt_totalsize( pDtb );
    int errCode = fdt_check_full( pDtb, totalSz );
    if( errCode ) {
        // print
        hcf();
    }

    int firstFoundUartNodeOffset = -1;
    for( int currentNodeOffest = 0; currentNodeOffest >= 0; )
    {
        const char* pCompatible = (const char*)fdt_getprop( pDtb, currentNodeOffest, "compatible", NULL );

        bool matchedUartProtocol = false;
        for( int i = 0; i < sizeof( UART_ACCEPTED ); ++i )
        {
            const char* uartProtocol = UART_ACCEPTED[i];
            if( strncmp( pCompatible, uartProtocol, sizeof( uartProtocol ) ) == 0 )
            {
                matchedUartProtocol = true;
                break;
            }
        }
        const char* pStatus = (const char*)fdt_getprop( pDtb, currentNodeOffest, "status", NULL );
        if( ( strncmp( pStatus, DEVICE_DTB_OKAY, sizeof( DEVICE_DTB_OKAY ) ) == 0 ) 
            || ( strncmp( pStatus, DEVICE_DTB_OK, sizeof( DEVICE_DTB_OK ) ) == 0 ) )
        {
            firstFoundUartNodeOffset = currentNodeOffest;
            break;
        }

        currentNodeOffest = fdt_next_node( pDtb, currentNodeOffest, NULL );
    }

    if( firstFoundUartNodeOffset == -1 )
    {
        // Print
        hcf();
    }

    int parentOffset = fdt_parent_offset( pDtb, firstFoundUartNodeOffset );
    int addrCells = fdt_address_cells( pDtb, parentOffset );
    if( addrCells > 2 )
    {
        //print we're a 64 bits kernel so our addr space is 64bits, at most 2 FDT addrCells
        hcf();
    }
    int sizeCells = fdt_size_cells( pDtb, parentOffset );
    if( sizeCells > 2 )
    {
        //print we're a 64 bits kernel so our addr space is 64bits, at most 2 FDT sizeCells
        hcf();
    }
  
    const uint32_t* pReg = (const uint32_t*) fdt_getprop( pDtb, firstFoundUartNodeOffset, "reg", NULL );
    uint16_t regShift = *(const uint16_t*) fdt_getprop( pDtb, firstFoundUartNodeOffset, "reg-shift", NULL );
    uint16_t regIOWidth = *(const uint16_t*) fdt_getprop( pDtb, firstFoundUartNodeOffset, "reg-io-width", NULL );

    uint64_t baseAddr = ( (uint64_t) ( fdt32_to_cpu( pReg[0] ) ) << 32 ) | ( (uint64_t) fdt32_to_cpu( pReg[1] ) );
    // NOTE: it's absurd for the size to exceed even unit32
    uint32_t size = fdt32_to_cpu( pReg[3] );

    uart_port serialComPort = {
        .baseAddr = baseAddr,
        .size = size,
        .regShift = fdt16_to_cpu( regShift ),
        .regIOWidthInBytes = fdt16_to_cpu( regIOWidth )
    };
}

// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void KernelMain() 
{
    InitLimineRequests();
    InitFramebuffer();
    InitFbConsole();

    QPrint( "Hello Quakernel !", 0xff5555 );

    // We're done, just hang...
    hcf();
}
