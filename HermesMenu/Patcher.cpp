#include "Patcher.h"
#include "External.h"

#include <xtl.h>

#define KERNEL_STORE_ADDRESS 0x8000000C // this is address in kernel we store the address of our allocated memory block
#define KERNEL_START_ADDRESS 0x80010000 // base addy of kernel
#define KERNEL_ALLOCATE_ADDRESS 0x7FFD2200 // where we want to put our allocated memory block (under kernel so it works retail)
#define KERNEL_SEARCH_RANGE 0x02AF90 // used for loop control base + search range to look xbe entry point bytes

#define ETM_HEAP_SIZE 2400  // just enough room to match evox's etm spec limit (no need to give them more room then evox does)

#define SIZEOFLOADERDATA 167
static unsigned char trainerloaderdata[SIZEOFLOADERDATA] = {
    0x60, 0xBA, 0x34, 0x12, 0x00, 0x00, 0x60, 0x6A, 0x01, 0x6A, 0x07, 0xE8, 0x67, 0x00, 0x00, 0x00,
    0x6A, 0x0C, 0x6A, 0x08, 0xE8, 0x5E, 0x00, 0x00, 0x00, 0x61, 0x8B, 0x35, 0x18, 0x01, 0x01, 0x00,
    0x83, 0xC6, 0x08, 0x8B, 0x06, 0x8B, 0x72, 0x12, 0x03, 0xF2, 0xB9, 0x03, 0x00, 0x00, 0x00, 0x3B,
    0x06, 0x74, 0x0C, 0x83, 0xC6, 0x04, 0xE2, 0xF7, 0x68, 0xF0, 0x00, 0x00, 0x00, 0xEB, 0x29, 0x8B,
    0xEA, 0x83, 0x7A, 0x1A, 0x00, 0x74, 0x05, 0x8B, 0x4A, 0x1A, 0xEB, 0x03, 0x8B, 0x4A, 0x16, 0x03,
    0xCA, 0x0F, 0x20, 0xC0, 0x50, 0x25, 0xFF, 0xFF, 0xFE, 0xFF, 0x0F, 0x22, 0xC0, 0xFF, 0xD1, 0x58,
    0x0F, 0x22, 0xC0, 0x68, 0xFF, 0x00, 0x00, 0x00, 0x6A, 0x08, 0xE8, 0x08, 0x00, 0x00, 0x00, 0x61,
    0xFF, 0x15, 0x28, 0x01, 0x01, 0x00, 0xC3, 0x55, 0x8B, 0xEC, 0x66, 0xBA, 0x04, 0xC0, 0xB0, 0x20,
    0xEE, 0x66, 0xBA, 0x08, 0xC0, 0x8A, 0x45, 0x08, 0xEE, 0x66, 0xBA, 0x06, 0xC0, 0x8A, 0x45, 0x0C,
    0xEE, 0xEE, 0x66, 0xBA, 0x02, 0xC0, 0xB0, 0x1A, 0xEE, 0x50, 0xB8, 0x40, 0x42, 0x0F, 0x00, 0x48,
    0x75, 0xFD, 0x58, 0xC9, 0xC2, 0x08, 0x00,
};

bool Patcher::InstallTrainer(uint8_t* trainerData, uint32_t trainerSize)
{

  bool Found = false;

  uint8_t* xboxkrnl = (uint8_t*)KERNEL_START_ADDRESS;
  uint8_t* hackptr = (uint8_t*)KERNEL_STORE_ADDRESS;
  void *ourmemaddr = NULL; // pointer used to allocated trainer mem
  unsigned int i = 0;
  DWORD memsize = ETM_HEAP_SIZE;

  uint8_t xbe_entry_point[] = {0xff,0x15,0x28,0x01,0x01,0x00}; // xbe entry point bytes in kernel
  uint8_t evox_tsr_hook[] = {0xff,0x15,0x10,0x00,0x00,0x80}; // check for evox's evil tsr hook

  for(i = 0; i < KERNEL_SEARCH_RANGE; i++)
  {
    if (memcmp(&xboxkrnl[i], xbe_entry_point, sizeof(xbe_entry_point)) == 0 ||
      memcmp(&xboxkrnl[i], evox_tsr_hook, sizeof(evox_tsr_hook)) == 0)
    {
      Found = true;
      break;
    }
  }

  if(Found)
  {
    uint8_t* patchlocation = xboxkrnl;

    patchlocation += i + 2; // adjust to xbe entry point bytes in kernel (skipping actual call opcodes)
    _asm
    {
      pushad

      mov eax, cr0
      push eax
      and eax, 0FFFEFFFFh
      mov cr0, eax // disable memory write prot

      mov	edi, patchlocation // address of call to xbe entry point in kernel
      mov	dword ptr [edi], KERNEL_STORE_ADDRESS // patch with address of where we store loaderdata+trainer buffer address

      pop eax
      mov cr0, eax // restore memory write prot

      popad
    }
  }
  else
  {
    __asm // recycle check
    {
      pushad

      mov edx, KERNEL_STORE_ADDRESS
      mov ecx, DWORD ptr [edx]
      cmp ecx, 0 // just in case :)
      jz cleanup

      cmp word ptr [ecx], 0BA60h // address point to valid loaderdata?
      jnz cleanup

      mov Found, 1 // yes! flag it found

      push ecx
      call MmFreeContiguousMemory // release old memory
cleanup:
      popad
    }
  }

  // allocate our memory space BELOW the kernel (so we can access buffer from game's scope)
  // if you allocate above kernel our buffer is out of scope and only debug bio will allow
  // game to access it
  ourmemaddr = MmAllocateContiguousMemoryEx(memsize, 0, -1, KERNEL_ALLOCATE_ADDRESS, PAGE_NOCACHE | PAGE_READWRITE);
  if ((DWORD)ourmemaddr > 0)
  {
    MmPersistContiguousMemory(ourmemaddr, memsize, true); // so we survive soft boots
    memcpy(hackptr, &ourmemaddr, 4); // store location of ourmemaddr in kernel

    memset(ourmemaddr, 0xFF, memsize); // init trainer buffer
    memcpy(ourmemaddr, trainerloaderdata, sizeof(trainerloaderdata)); // copy loader data (actual kernel hack)

    // patch loaderdata with trainer base address
    _asm
    {
      pushad

      mov eax, ourmemaddr
      mov ebx, eax
      add ebx, SIZEOFLOADERDATA
      mov dword ptr [eax+2], ebx

      popad
    }

		// adjust ourmemaddr pointer past loaderdata
		ourmemaddr=(PVOID *)(((unsigned int) ourmemaddr) + sizeof(trainerloaderdata));

		// copy our trainer data into allocated mem
		memcpy(ourmemaddr, trainerData, trainerSize);

  }

	return Found;
}

bool Patcher::RemoveTrainer()
{
	bool Found = false;
    uint8_t* xboxkrnl = (uint8_t*)KERNEL_START_ADDRESS;
    unsigned int i = 0;

  unsigned char xbe_entry_point[] = {0xff,0x15,0x80,0x00,0x00,0x0c}; // xbe entry point bytes in kernel
  *((DWORD*)(xbe_entry_point+2)) = KERNEL_STORE_ADDRESS;

	for(i = 0; i < KERNEL_SEARCH_RANGE; i++)
	{
		if (memcmp(&xboxkrnl[i], xbe_entry_point, 6) == 0)
		{
			Found = true;
			break;
		}
	}

	if(Found)
  {
    uint8_t* patchlocation = xboxkrnl;
    patchlocation += i + 2; // adjust to xbe entry point bytes in kernel (skipping actual call opcodes)
    __asm // recycle check
    {
        pushad

        mov eax, cr0
        push eax
        and eax, 0FFFEFFFFh
        mov cr0, eax // disable memory write prot

        mov	edi, patchlocation // address of call to xbe entry point in kernel
        mov	dword ptr [edi], 0x00010128 // patch with address of where we store loaderdata+trainer buffer address

        pop eax
        mov cr0, eax // restore memory write prot

        popad
    }
  }
  return Found;
}