
# Copilot Instructions for OpenSeaChest

## Project Overview

openSeaChest is a cross-platform C project for managing storage devices (HDDs and SSDs) attached via ATA, SATA, SCSI, SAS, NVMe, USB, and FireWire interfaces. It provides libraries and command-line tools for low-level drive operations, diagnostics, health monitoring, firmware updates, and secure erasure. The project supports ~200 commands and operations across 18 specialized utilities.

## Architecture Overview

### Library Stack (3-tier architecture, bottom to top):

1. **opensea-common** - OS-agnostic utilities and safe operations
   - Safe memory allocation (safe_malloc, safe_calloc, safe_realloc, safe_free)
   - Safe string operations (safe_strcpy, safe_strcat, safe_strtok, etc.)
   - Safe I/O operations (safe_fopen, safe_freopen, safe_tmpfile)
   - Safe conversion functions (safe_strtol, safe_atoi, safe_strtof, etc.)
   - Safe sorting/searching (safe_qsort, safe_bsearch, safe_lsearch, safe_lfind)
   - Safe character classification (safe_isalpha, safe_isdigit, safe_tolower, etc.)
   - Cross-platform timers, temperature conversions, bit manipulation
   - See full list of safe_ functions in sections below

2. **opensea-transport** - Low-level command transport and OS passthrough
   - ATA, SCSI, NVMe command construction and issuing
   - OS-specific passthrough implementations:
     - Windows: SCSI_PASS_THROUGH_DIRECT, IOCTL_IDE_PASS_THROUGH, STORAGE_PROTOCOL_COMMAND
     - Linux: SG_IO (SCSI Generic v3), NVMe kernel IOCTLs
     - BSD: CAM (Common Access Method) for SCSI/ATA, separate NVMe IOCTLs
     - Solaris/Illumos: USCSI for SCSI/ATA
   - SCSI translation for different interface types
   - USB/FireWire bridge handling and quirks

3. **opensea-operations** - High-level device operations
   - Common test sequences (SMART, DST, etc.)
   - Feature configuration and management
   - Encapsulates command set differences (ATA vs SCSI vs NVMe)
   - Firmware download, secure erase, format operations

### Command-Line Utilities (18 tools in utils/C/openSeaChest/):

Each utility focuses on specific operations:
- **openSeaChest_Basics** - Core diagnostics, device info, health checks
- **openSeaChest_Info** - Detailed device information
- **openSeaChest_SMART** - SMART data and health monitoring
- **openSeaChest_Logs** - Log retrieval and parsing
- **openSeaChest_Firmware** - Firmware download and update
- **openSeaChest_Erase** - Data sanitization and secure erase
- **openSeaChest_Format** - Low-level formatting
- **openSeaChest_Configure** - Feature configuration
- **openSeaChest_Security** - Security features (ATA Security, TCG)
- **openSeaChest_PowerControl** - Power management
- **openSeaChest_Defect** - Defect management (SCSI)
- **openSeaChest_GenericTests** - Generic testing operations
- **openSeaChest_NVMe** - NVMe-specific operations
- **openSeaChest_Raw** - Raw command passthrough
- **openSeaChest_PassthroughTest** - USB compatibility testing
- **openSeaChest_Reservations** - SCSI reservations
- **openSeaChest_ZBD** - Zoned Block Device operations
- **openSeaChest_Sample** - Example/template utility

When generating code, consider which layer and utility the code belongs to.

## Storage Device Types & Interfaces

### Command Sets (Protocol Layer):

- **ATA/SATA** - Consumer/enterprise HDDs and SSDs
  - 28-bit LBA (legacy, limited to 128GB)
  - 48-bit LBA (modern, supports >2TB)
  - ATA Security, SMART, SCT commands
  
- **SCSI/SAS** - Enterprise HDDs, SSDs, and arrays
  - More feature-rich than ATA
  - Different error handling and logging structures
  - Advanced features: defect management, reservations, extensive mode pages
  
- **NVMe** - Modern SSDs with PCIe interface
  - Very different command structure from ATA/SCSI
  - Admin and I/O command sets
  - Namespace management, telemetry, sanitize

### Transport Interfaces:

- **Direct Attach:** SATA, SAS, NVMe (direct OS passthrough)
- **USB:** Requires USB bridge translation (SAT for ATA/SCSI, UASP)
  - Many limitations and quirks per bridge chip manufacturer
  - Use openSeaChest_PassthroughTest for compatibility validation
  - Some commands may not work through certain USB bridges
- **FireWire:** SBP-2 protocol (legacy support)

### Platform-Specific Passthrough:

- **Windows:**
  - SCSI: SCSI_PASS_THROUGH_DIRECT, IOCTL_IDE_PASS_THROUGH
  - NVMe: STORAGE_PROTOCOL_COMMAND (Win10+), vendor-specific IOCTLs (Win8.1/7)
  
- **Linux:**
  - SG_IO (SCSI Generic, v3 implemented)
  - libATA blocks ATA security commands by default (kernel parameter to override)
  - NVMe kernel IOCTLs
  
- **BSD (FreeBSD, OpenBSD, NetBSD, DragonFlyBSD):**
  - CAM (Common Access Method) for SCSI/ATA
  - Separate NVMe IOCTLs (FreeBSD)
  - OpenBSD/NetBSD: ATA passthrough limited to 28-bit commands
  
- **Solaris/Illumos:**
  - USCSI for SCSI/ATA
  - UNVME for NVMe (partial support)

When writing device code, always consider:
1. Which command set is in use (ATA vs SCSI vs NVMe)?
2. What OS passthrough mechanism is available?
3. Are there bridge translation layers (USB/FireWire)?

## Language and Standards

- **C Standards**: All C code must work with C99/C11 and later standards
- **C++ Usage**: Use C++ only where explicitly required
- **Formatting**: Use .clang-format file for formatting rules
- **Style Guide**: See CONTRIBUTING.md for detailed coding conventions

## opensea-common Safe Functions (ALWAYS USE THESE)

The opensea-common library provides bounds-checked, cross-platform safe alternatives to standard C library functions. These implement C11 Annex K style bounds checking and follow ISO/IEC TS 17961 (C Secure Coding Rules).

### Memory Allocation (ALWAYS use instead of malloc/calloc/realloc/free):

```c
// Allocate memory with overflow and zero-size protection
void* safe_malloc(size_t size);
void* safe_calloc(size_t count, size_t size);
void* safe_realloc(void* block, size_t size);
void* safe_reallocf(void** block, size_t size);  // Frees original on failure

// Free memory and NULL the pointer (prevents double-free)
safe_free(&pointer);  // Generic macro, auto-selects correct type helper
// Type-specific helpers also available:
// safe_free_char, safe_free_uchar, safe_free_int, safe_free_uint, etc.

// Aligned memory allocation
void* safe_aligned_malloc(size_t alignment, size_t size);
void* safe_aligned_calloc(size_t alignment, size_t count, size_t size);
void* safe_aligned_realloc(void* block, size_t alignment, size_t size);
safe_free_aligned(&pointer);

// Page-aligned memory allocation
void* safe_page_aligned_malloc(size_t size);
void* safe_page_aligned_calloc(size_t count, size_t size);
void* safe_page_aligned_realloc(void* block, size_t size);
safe_free_page_aligned(&pointer);
```

### String Operations (ALWAYS use instead of strcpy/strcat/strtok/etc.):

```c
// String length (safe versions)
size_t safe_strlen(const char* string);
size_t safe_strnlen(const char* string, size_t n);

// String copy (bounds checked, prevents truncation)
errno_t safe_strcpy(char* dest, rsize_t destsz, const char* src);
errno_t safe_strncpy(char* dest, rsize_t destsz, const char* src, rsize_t count);
errno_t safe_strmove(char* dest, rsize_t destsz, const char* src);  // Allows overlap
errno_t safe_strnmove(char* dest, rsize_t destsz, const char* src, rsize_t count);

// String concatenation (bounds checked)
errno_t safe_strcat(char* dest, rsize_t destsz, const char* src);
errno_t safe_strncat(char* dest, rsize_t destsz, const char* src, rsize_t count);

// String tokenization (bounds checked)
char* safe_strtok(char* str, rsize_t strmax, const char* delim, char** saveptr);
#define safe_String_Token safe_strtok  // Alias

// String duplication (safe allocation)
errno_t safe_strdup(char** dup, const char* src);
errno_t safe_strndup(char** dup, const char* src, size_t len);
```

### Memory Operations (ALWAYS use instead of memset/memcpy/memmove):

```c
errno_t safe_memset(void* dest, rsize_t destsz, int ch, rsize_t count);
errno_t safe_memcpy(void* dest, rsize_t destsz, const void* src, rsize_t count);
errno_t safe_memmove(void* dest, rsize_t destsz, const void* src, rsize_t count);
int safe_memcmp(const void* s1, rsize_t s1max, const void* s2, rsize_t s2max, rsize_t n);

// Secure zero (prevents compiler optimization, ensures data is wiped)
errno_t secure_memzero(void* dest, rsize_t destsz);
```

### String to Number Conversion (ALWAYS use instead of strtol/atoi/etc.):

These use output parameters and perform all error checking internally:

```c
// Integer conversions (base-aware)
errno_t safe_strtol(long* value, const char* str, char** endp, int base);
errno_t safe_strtoll(long long* value, const char* str, char** endp, int base);
errno_t safe_strtoul(unsigned long* value, const char* str, char** endp, int base);
errno_t safe_strtoull(unsigned long long* value, const char* str, char** endp, int base);
errno_t safe_strtoimax(intmax_t* value, const char* str, char** endp, int base);
errno_t safe_strtoumax(uintmax_t* value, const char* str, char** endp, int base);

// Decimal conversions (base 10 only)
errno_t safe_atoi(int* value, const char* str);
errno_t safe_atol(long* value, const char* str);
errno_t safe_atoll(long long* value, const char* str);

// Floating-point conversions
errno_t safe_strtof(float* value, const char* str, char** endp);
errno_t safe_strtod(double* value, const char* str, char** endp);
errno_t safe_strtold(long double* value, const char* str, char** endp);
errno_t safe_atof(double* value, const char* str);
```

### File I/O (ALWAYS use instead of fopen/freopen/tmpfile/etc.):

```c
errno_t safe_fopen(FILE** streamptr, const char* filename, const char* mode);
errno_t safe_freopen(FILE** newstreamptr, const char* filename, const char* mode, FILE* stream);
errno_t safe_tmpnam(char* filename_s, rsize_t maxsize);
errno_t safe_tmpfile(FILE** streamptr);
char* safe_gets(char* str, rsize_t n);
```

### Character Classification (use instead of isalpha/isdigit/etc.):

```c
// Safe versions that check for valid input range
int safe_isascii(int c);
int safe_isalnum(int c);
int safe_isalpha(int c);
int safe_isblank(int c);
int safe_iscntrl(int c);
int safe_isdigit(int c);
int safe_isgraph(int c);
int safe_islower(int c);
int safe_isprint(int c);
int safe_ispunct(int c);
int safe_isspace(int c);
int safe_isupper(int c);
int safe_isxdigit(int c);
int safe_tolower(int c);
int safe_toupper(int c);
```

### Sorting and Searching (use instead of qsort/bsearch):

```c
// Sorting with bounds checking
errno_t safe_qsort(void* ptr, rsize_t count, rsize_t size, comparefn compare);
errno_t safe_qsort_context(void* ptr, rsize_t count, rsize_t size, 
                           ctxcomparefn compare, void* context);

// Searching with bounds checking
void* safe_bsearch(const void* key, const void* ptr, rsize_t count, 
                   rsize_t size, comparefn compare);
void* safe_bsearch_context(const void* key, void* ptr, rsize_t count, 
                           rsize_t size, ctxcomparefn compare, void* context);

// Linear search
void* safe_lsearch(const void* key, void* base, rsize_t* nelp, 
                   rsize_t width, comparefn compar);
void* safe_lsearch_context(const void* key, void* base, rsize_t* nelp, 
                           rsize_t width, ctxcomparefn compar, void* context);

// Linear find
void* safe_lfind(const void* key, const void* base, rsize_t* nelp, 
                 rsize_t width, comparefn compar);
void* safe_lfind_context(const void* key, const void* base, rsize_t* nelp, 
                         rsize_t width, ctxcomparefn compar, void* context);
```

**CRITICAL**: Always use these safe_ functions instead of their standard C library equivalents. They provide:
- Bounds checking similar to C11 Annex K
- Protection against overflow in size calculations
- Prevention of zero-size allocations
- Cross-platform compatibility
- ISO/IEC TS 17961 (C Secure Coding Rules) compliance

## Common Coding Patterns

### Device Access Pattern:

```c
#include "common.h"
#include "common_public.h"

tDevice device;
safe_memset(&device, 0, sizeof(tDevice));

// Initialize device handle
if (SUCCESS != get_Device(deviceHandle, &device)) {
    // Handle error
    return FAILURE;
}

// Perform operations
int result = perform_operation(&device);

// Always close device
close_Device(&device);
```

### Error Handling:

```c
// Use project error codes from common.h
// Return values: SUCCESS, FAILURE, NOT_SUPPORTED, IN_PROGRESS, PERMISSION_DENIED, etc.

int ret = some_operation(&device);
if (ret != SUCCESS) {
    if (ret == NOT_SUPPORTED) {
        // Feature not available on this device/interface
        if (VERBOSITY_QUIET < device.deviceVerbosity) {
            printf("Operation not supported on this device.\n");
        }
    } else if (ret == PERMISSION_DENIED) {
        // Need elevated privileges
        if (VERBOSITY_QUIET < device.deviceVerbosity) {
            printf("Operation requires administrator/root privileges.\n");
        }
    } else {
        // General failure
        if (VERBOSITY_QUIET < device.deviceVerbosity) {
            printf("Operation failed with error code %d.\n", ret);
        }
    }
    return ret;
}
```

### Memory Allocation:

```c
// ALWAYS use safe_* functions from opensea-common
uint8_t *buffer = C_CAST(uint8_t*, safe_calloc(bufferSize, sizeof(uint8_t)));
if (!buffer) {
    return MEMORY_FAILURE;
}

// Use buffer for operations...

// Always free (safe_free takes pointer-to-pointer and NULLs it)
safe_free(&buffer);
```

### String Operations:

```c
char destination[256];
safe_memset(destination, 0, sizeof(destination));

// Copy with bounds checking
errno_t err = safe_strcpy(destination, sizeof(destination), source);
if (err != 0) {
    // Handle truncation or error
    return FAILURE;
}

// Concatenate with bounds checking
err = safe_strcat(destination, sizeof(destination), " suffix");
if (err != 0) {
    // Handle error
    return FAILURE;
}
```

### String to Number Conversion:

```c
long value = 0;
char* endptr = NULL;
errno_t err = safe_strtol(&value, inputString, &endptr, 10);
if (err != 0) {
    // Conversion failed (invalid input, overflow, etc.)
    return FAILURE;
}
if (endptr == inputString) {
    // No digits were converted
    return FAILURE;
}
```

### Endianness Handling:

```c
// Use byte_Swap_* macros for endianness conversion
uint32_t value = byte_Swap_32(raw_value);  // Big-endian to host
uint16_t word = byte_Swap_16(raw_word);    // Big-endian to host
```

### Verbose Output:

```c
// Use verbosity levels for debugging output
if (VERBOSITY_COMMAND_VERBOSE <= device.deviceVerbosity) {
    printf("Detailed operation information: sending command 0x%02X\n", commandCode);
}

if (VERBOSITY_BUFFERS <= device.deviceVerbosity) {
    // Print buffer dumps for deepest debugging
    print_Data_Buffer(buffer, bufferSize, true);
}
```

## Cross-Platform Compatibility

### Platform Detection:

Use preprocessor directives for platform-specific code:

```c
#if defined(_WIN32)
    // Windows-specific code
#elif defined(__linux__)
    // Linux-specific code
#elif defined(__FreeBSD__)
    // FreeBSD-specific code
#elif defined(__NetBSD__)
    // NetBSD-specific code
#elif defined(__OpenBSD__)
    // OpenBSD-specific code
#elif defined(__DragonFly__)
    // DragonFlyBSD-specific code
#elif defined(__sun)
    // Solaris/Illumos-specific code
#endif
```

### Platform Support Matrix:

See README.md for detailed compatibility tables covering:
- SAS/SATA support across all platforms
- NVMe support (varies by platform and OS version)
- Platform-specific limitations (e.g., NetBSD/OpenBSD limited to 28-bit ATA)

## Testing Requirements

### Manual Testing:

When adding new features or fixing bugs:
1. Test on multiple device types (HDD, SSD) if applicable
2. Test on multiple interfaces (SATA, USB) when possible
3. Test on target OS platforms (Windows, Linux minimum)
4. Verify verbose output provides useful diagnostic information (`-v 3` or `-v 4`)
5. Ensure error conditions are handled gracefully
6. Test with both direct-attached and USB devices if applicable

### USB Device Compatibility:

USB devices require special attention due to bridge chip variability:
- Use `openSeaChest_PassthroughTest` to validate new USB devices
- Document bridge chip compatibility issues in bug reports
- Some commands may not work through certain USB bridges (especially security/firmware operations)
- Provide clear error messages when USB limitations are encountered

### Cross-Platform Validation:

- GitHub Actions CI runs on:
  - Windows (MSVC x64/x86/ARM64, GCC, Clang)
  - Linux (Ubuntu GCC/Clang)
  - MUSL cross-compile (8 architectures)
- Cirrus CI provides FreeBSD testing
- VMActions provide OpenBSD, NetBSD, DragonFlyBSD, Solaris, Illumos testing

## Performance Considerations

### Avoid Unnecessary Device Queries:

Device identification queries can be slow (especially on USB). Cache results in the tDevice structure when possible rather than re-querying.

### Large Data Transfers:

For operations involving large data transfers (firmware download, read/write operations):
- Use appropriate buffer sizes (typically 512B-1MB depending on operation)
- Consider drive sector size (512B native vs 4K native)
- Show progress indicators for long operations (use --progress flag when available)
- Respect device timeout values (may need to increase for slow devices)

### USB Performance:

USB devices are significantly slower than direct-attached drives:
- Provide appropriate timeouts (2-3x longer than SATA)
- Give user feedback for long operations
- Some USB bridges have command limitations that require workarounds

## Error Handling

### Project Error Codes:

Use error codes from `common.h` for consistency:
- `SUCCESS` (0)
- `FAILURE` (1)
- `NOT_SUPPORTED` (2)
- `IN_PROGRESS` (3)
- `ABORTED` (4)
- `BAD_PARAMETER` (5)
- `MEMORY_FAILURE` (6)
- `OS_PASSTHROUGH_FAILURE` (7)
- `PERMISSION_DENIED` (8)
- Many others defined in the codebase

Return appropriate error codes and handle them at each level.

## Security Requirements

Follow secure coding standards from:
- **ISO/IEC TS 17961** (C Secure Coding Rules)
- **CERT C Coding Standard**
- **OWASP** (secure coding practices)
- **OpenSSF** (Open Source Security Foundation best practices)

Key security practices:
- Always use safe_ functions for memory, strings, I/O, and conversions
- Validate all inputs, especially from command-line arguments
- Check bounds before array/buffer access
- Prevent integer overflows in size calculations
- Use secure_memzero() for sensitive data (prevents compiler optimization)
- Handle errors from all operations
- Follow principle of least privilege

## Documentation Standards

- **Comment Style**: Use doxygen-style comments for all public functions
- **Function Documentation**: Include purpose, parameters, return values, and notes
- **Code Comments**: Explain WHY, not WHAT (code should be self-documenting)
- **Complex Logic**: Document algorithms, especially non-obvious optimizations

Example:
```c
//! \fn int my_function(tDevice* device, uint32_t value)
//! \brief Performs operation X on the device
//! \param device pointer to device structure
//! \param value input value for operation
//! \return SUCCESS on success, FAILURE on error, NOT_SUPPORTED if device doesn't support operation
int my_function(tDevice* device, uint32_t value)
{
    // Implementation
}
```

## Additional Documentation References

For comprehensive information, consult:
- **README.md** - Project overview, 200+ commands, platform compatibility tables
- **CONTRIBUTING.md** - Detailed contribution guidelines, commit message format, code style
- **BUILDING.md** - Build instructions for all platforms (Meson, Make, MSVC)
- **docs/** - Design documentation:
  - `OPTION_FRAMEWORK_PROPOSAL.md` - Option parsing architecture redesign
  - `MIGRATION_PLAN.md` - Ongoing refactoring context
  - `MIGRATION_STATUS.md` - Current refactoring status
- **Subproject Documentation:**
  - `subprojects/opensea-common/README.md` - Common utilities documentation
  - `subprojects/opensea-common/CONTRIBUTING.md` - Common library coding standards
  - `subprojects/opensea-transport/README.md` - Transport layer documentation
  - `subprojects/opensea-transport/CONTRIBUTING.md` - Transport library coding standards
  - `subprojects/opensea-operations/README.md` - Operations layer documentation
  - `subprojects/opensea-operations/CONTRIBUTING.md` - Operations library coding standards

When reviewing or modifying code in subprojects, check their individual CONTRIBUTING.md files for library-specific requirements.

## Summary

This is a complex, security-focused, cross-platform storage device management project with:
- 18 command-line utilities covering 200+ operations
- 3-tier library architecture (common, transport, operations)
- Support for multiple device types (HDD, SSD) and interfaces (ATA, SCSI, NVMe, USB, FireWire)
- Support for 10+ operating systems and variants
- Extensive safe_ function library for secure memory, string, I/O, and conversion operations
- Active migration/refactoring to improve architecture

Always prioritize security, cross-platform compatibility, and robust error handling. Use the safe_ functions from opensea-common for all memory, string, I/O, and conversion operations.
