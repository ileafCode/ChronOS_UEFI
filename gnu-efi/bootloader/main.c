#include <efi.h>
#include <efilib.h>
#include <elf.h>

#include <stdint.h>

typedef struct {
	void* BaseAddress;
	uint64_t BufferSize;
	uint32_t Width;
	uint32_t Height;
	uint32_t PixelsPerScanLine;
} Framebuffer;

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

typedef struct {
	unsigned char magic[2];
	unsigned char mode;
	unsigned char charsize;
} PSF1_HEADER;

typedef struct {
	PSF1_HEADER* psf1_Header;
	void* glyphBuffer;
} PSF1_FONT;

typedef struct {
    uint32_t length;
    uint16_t version;
    uint32_t header_length;
    uint8_t min_instruction_length;
    uint8_t default_is_stmt;
    int8_t line_base;
    uint8_t line_range;
    uint8_t opcode_base;
    uint8_t std_opcode_lengths[12];
} __attribute__((packed)) DebugLineHeader;

typedef struct {
	Framebuffer *framebuffer;
	PSF1_FONT *psf1_Font;
	EFI_MEMORY_DESCRIPTOR *mMap;
	UINTN mMapSize;
	UINTN mMapDescSize;
	UINT64 rsdp;
} boot_info_t;

typedef struct {
    CHAR8  Signature[8]; // "RSD PTR "
    UINT8  Checksum;
    CHAR8  OEMID[6];
    UINT8  Revision;
    UINT32 RsdtAddress;
    // ACPI 2.0+ fields:
    UINT32 Length;
    UINT64 XsdtAddress;
    UINT8  ExtendedChecksum;
    UINT8  Reserved[3];
} RSDPDescriptor;

BOOLEAN ValidateRSDP(RSDPDescriptor *rsdp) {
    UINT8 sum = 0;
    UINTN length = (rsdp->Revision < 2) ? 20 : rsdp->Length;

    // Checksum over the entire structure (20 bytes for ACPI 1.0, or rsdp->Length for ACPI 2.0+)
    for (UINTN i = 0; i < length; i++) {
        sum += ((UINT8*)rsdp)[i];
    }
	Print(L"Sum: %d\n", sum);
    return (sum == 0);
}

Framebuffer framebuffer;
Framebuffer* InitializeGOP(){
	EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
	EFI_STATUS status;

	status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);
	if(EFI_ERROR(status)){
		Print(L"Unable to locate GOP\n\r");
		return NULL;
	}
	else
	{
		Print(L"GOP located\n\r");
	}

	framebuffer.BaseAddress = (void*)gop->Mode->FrameBufferBase;
	framebuffer.BufferSize = gop->Mode->FrameBufferSize;
	framebuffer.Width = gop->Mode->Info->HorizontalResolution;
	framebuffer.Height = gop->Mode->Info->VerticalResolution;
	framebuffer.PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;

	return &framebuffer;
	
}

EFI_FILE* LoadFile(EFI_FILE* Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable){
	EFI_FILE* LoadedFile;

	EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;
	SystemTable->BootServices->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&LoadedImage);

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem;
	SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&FileSystem);

	if (Directory == NULL){
		FileSystem->OpenVolume(FileSystem, &Directory);
	}

	EFI_STATUS s = Directory->Open(Directory, &LoadedFile, Path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	if (s != EFI_SUCCESS){
		return NULL;
	}
	return LoadedFile;

}

PSF1_FONT* LoadPSF1Font(EFI_FILE* Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
	EFI_FILE* font = LoadFile(Directory, Path, ImageHandle, SystemTable);
	if (font == NULL) return NULL;

	PSF1_HEADER* fontHeader;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_HEADER), (void**)&fontHeader);
	UINTN size = sizeof(PSF1_HEADER);
	font->Read(font, &size, fontHeader);

	if (fontHeader->magic[0] != PSF1_MAGIC0 || fontHeader->magic[1] != PSF1_MAGIC1){
		return NULL;
	}

	UINTN glyphBufferSize = fontHeader->charsize * 256;
	if (fontHeader->mode == 1) { //512 glyph mode
		glyphBufferSize = fontHeader->charsize * 512;
	}

	void* glyphBuffer;
	{
		font->SetPosition(font, sizeof(PSF1_HEADER));
		SystemTable->BootServices->AllocatePool(EfiLoaderData, glyphBufferSize, (void**)&glyphBuffer);
		font->Read(font, &glyphBufferSize, glyphBuffer);
	}

	PSF1_FONT* finishedFont;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_FONT), (void**)&finishedFont);
	finishedFont->psf1_Header = fontHeader;
	finishedFont->glyphBuffer = glyphBuffer;
	return finishedFont;

}

int memcmp(const void* aptr, const void* bptr, int n){
	const unsigned char* a = aptr, *b = bptr;
	for (int i = 0; i < n; i++){
		if (a[i] < b[i]) return -1;
		else if (a[i] > b[i]) return 1;
	}
	return 0;
}

int strncmp(const char *s1, const char *s2, register int n) {
	register unsigned char u1, u2;

	while (n-- > 0)
	{
		u1 = (unsigned char)*s1++;
		u2 = (unsigned char)*s2++;
		if (u1 != u2)
			return 0;
		if (u1 == '\0')
			return 1;
	}
	return 1;
}

EFI_STATUS efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	InitializeLib(ImageHandle, SystemTable);

	EFI_FILE* Kernel = LoadFile(NULL, L"kernel.elf", ImageHandle, SystemTable);
	if (Kernel == NULL){
		Print(L"Could not load kernel \n\r");
	}
	else{
		Print(L"Kernel Loaded Successfully \n\r");
	}

	Elf64_Ehdr header;
	{
		UINTN FileInfoSize;
		EFI_FILE_INFO* FileInfo;
		Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, NULL);
		SystemTable->BootServices->AllocatePool(EfiLoaderData, FileInfoSize, (void**)&FileInfo);
		Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, (void**)&FileInfo);

		UINTN size = sizeof(header);
		Kernel->Read(Kernel, &size, &header);
	}

	if (
		memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 ||
		header.e_ident[EI_CLASS] != ELFCLASS64 ||
		header.e_ident[EI_DATA] != ELFDATA2LSB ||
		header.e_type != ET_EXEC ||
		header.e_machine != EM_X86_64 ||
		header.e_version != EV_CURRENT
	)
	{
		Print(L"kernel format is bad\r\n");
	}
	else
	{
		Print(L"kernel header successfully verified\r\n");
	}

	Elf64_Phdr* phdrs;
	{
		Kernel->SetPosition(Kernel, header.e_phoff);
		UINTN size = header.e_phnum * header.e_phentsize;
		SystemTable->BootServices->AllocatePool(EfiLoaderData, size, (void**)&phdrs);
		Kernel->Read(Kernel, &size, phdrs);
	}

	for (
		Elf64_Phdr* phdr = phdrs;
		(char*)phdr < (char*)phdrs + header.e_phnum * header.e_phentsize;
		phdr = (Elf64_Phdr*)((char*)phdr + header.e_phentsize)
	)
	{
		switch (phdr->p_type){
			case PT_LOAD: {
				int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
				Elf64_Addr segment = phdr->p_paddr;
				SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, pages, (EFI_PHYSICAL_ADDRESS *)&segment);

				Kernel->SetPosition(Kernel, phdr->p_offset);
				UINTN size = phdr->p_filesz;
				Kernel->Read(Kernel, &size, (void*)segment);
				break;
			}
		}
	}

	Print(L"Kernel Loaded\n\r");
	
	void (*KernelStart)(boot_info_t *) = ((__attribute__((sysv_abi)) void (*)(boot_info_t*) ) header.e_entry);

	PSF1_FONT* newFont = LoadPSF1Font(NULL, L"font.psf", ImageHandle, SystemTable);
	if (newFont == NULL)
		Print(L"Font is not valid or is not found\n\r");
	else
		Print(L"Font found. char size = %d\n\r", newFont->psf1_Header->charsize);

	Framebuffer* newBuffer = InitializeGOP();

	Print(L"Base: 0x%x\n\rSize: 0x%x\n\rWidth: %d\n\rHeight: %d\n\rPixelsPerScanline: %d\n\r", 
	newBuffer->BaseAddress, 
	newBuffer->BufferSize, 
	newBuffer->Width, 
	newBuffer->Height, 
	newBuffer->PixelsPerScanLine);

	// Get the mempry map
	EFI_MEMORY_DESCRIPTOR *Map = NULL;
	UINTN MapSize = 0, MapKey = 0;
	UINTN DescriptorSize;
	UINT32 DescriptorVersion;
	EFI_STATUS status = SystemTable->BootServices->GetMemoryMap(&MapSize, Map, &MapKey, &DescriptorSize, &DescriptorVersion);
	if (status != EFI_BUFFER_TOO_SMALL) {
	    Print(L"Unexpected error when getting memory map size: %r\n", status);
	    while(1);
	}

	// Allocate a buffer that's a bit larger in case the map grows between calls.
	MapSize += DescriptorSize * 2;
	status = SystemTable->BootServices->AllocatePool(EfiLoaderData, MapSize, (void **)&Map);
	if (EFI_ERROR(status)) {
	    Print(L"Failed to allocate memory for the map: %r\n", status);
	    while(1);
	}

	// Get the memory map again with a proper buffer.
	status = SystemTable->BootServices->GetMemoryMap(&MapSize, Map, &MapKey, &DescriptorSize, &DescriptorVersion);
	if (EFI_ERROR(status)) {
	    Print(L"Failed to retrieve memory map: %r\n", status);
	    while(1);
	}

	EFI_CONFIGURATION_TABLE *configTable = SystemTable->ConfigurationTable;
	void *rsdp = NULL;
	EFI_GUID Acpi2TableGuid = ACPI_20_TABLE_GUID;

	for (UINTN index = 0; index < SystemTable->NumberOfTableEntries; index++) {
	    if (CompareGuid(&configTable[index].VendorGuid, &Acpi2TableGuid)) {
	        // Use the VendorTable of the current index.
	        CHAR8 *table = (CHAR8 *)configTable[index].VendorTable;
	        // Check that the first 8 bytes match "RSD PTR "
	        if (strncmp("RSD PTR ", (const char *)table, 8)) {
	            rsdp = (void *)table;
				if (ValidateRSDP((RSDPDescriptor *)rsdp)) {
	            	break;
				}
	        }
	    }
	}

	SystemTable->BootServices->GetMemoryMap(&MapSize, Map, &MapKey, &DescriptorSize, &DescriptorVersion);
	EFI_STATUS bs = SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);
	if (bs != EFI_SUCCESS) {
		Print(L"Failed to exit boot services: %x", bs);
		while (1);
	}

	boot_info_t boot_info;
	boot_info.framebuffer = &framebuffer;
	boot_info.mMap = Map;
	boot_info.mMapSize = MapSize;
	boot_info.mMapDescSize = DescriptorSize;
	boot_info.psf1_Font = newFont;
	boot_info.rsdp = (UINT64)rsdp;

	__asm__ ("xorq %rbp, %rbp");
	KernelStart(&boot_info);

	return EFI_SUCCESS; // Exit the UEFI application
}