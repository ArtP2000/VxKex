#include "buildcfg.h"
#include "kxbasep.h"

KXBASEAPI PVOID VirtualAlloc2(HANDLE Process, PVOID BaseAddress, SIZE_T Size, ULONG AllocationType,
					   ULONG PageProtection, MEM_EXTENDED_PARAMETER* ExtendedParameters, ULONG ParameterCount)
/*
  This set of functions makes use of *Ex variants of NtAllocateVirtualMemory/NtMapViewOfSection/NtUnmapViewOfSection introduced
  in Windows 10, allowing them to take advantage of new features such as specifying the NUMA node from which to allocate memory,
  "placeholders" and bounds on where allocated memory can be located.

  These features aren't particularly necessary for now, so we are using the older variants of these functions instead of 
  potentially modifying the OS' memory management.
*/
{
	NTSTATUS Status;
	PVOID NewAddress = BaseAddress;
	SIZE_T RegionSize = Size;

	if(AllocationType & MEM_RESERVE_PLACEHOLDER)
	{
		AllocationType ^= MEM_RESERVE_PLACEHOLDER;
		AllocationType |= MEM_RESERVE;
	}

	if(AllocationType & MEM_REPLACE_PLACEHOLDER)
	{
		AllocationType ^= MEM_REPLACE_PLACEHOLDER;
	}

	if(AllocationType & MEM_RESET_UNDO)
	{
		AllocationType ^= MEM_RESET_UNDO;
	}

	Status = NtAllocateVirtualMemory(Process, &NewAddress, NULL, &RegionSize, AllocationType, PageProtection);

	
	if(Status < STATUS_SUCCESS)
	{
		BaseSetLastNTError(Status);
		return FALSE;
	}

	return NewAddress;


}

KXBASEAPI PVOID MapViewOfFile3(HANDLE FileMapping, HANDLE Process, PVOID BaseAddress, ULONG64 Offset, SIZE_T ViewSize, ULONG AllocationType,
					   ULONG PageProtection, MEM_EXTENDED_PARAMETER* ExtendedParameters, ULONG ParameterCount)
{
	NTSTATUS Status;
	PVOID NewAddress = BaseAddress;
	SIZE_T RegionSize = ViewSize;

	if(AllocationType & MEM_REPLACE_PLACEHOLDER)
	{
		AllocationType ^= MEM_REPLACE_PLACEHOLDER;
	}

	Status = NtMapViewOfSection(FileMapping, Process, &NewAddress, NULL, NULL, Offset, &RegionSize, 0, AllocationType, PageProtection);

	if(Status < STATUS_SUCCESS)
	{
		BaseSetLastNTError(Status);
		return FALSE;
	}

	return NewAddress;

}


KXBASEAPI BOOL UnmapViewOfFile2(HANDLE Process, PVOID BaseAddress, ULONG UnmapFlags)
{
	NTSTATUS Status;

	Status = NtUnmapViewOfSection(Process, BaseAddress);

	if(Status < STATUS_SUCCESS)
	{
		BaseSetLastNTError(Status);
		return FALSE;
	}

	return TRUE;

}