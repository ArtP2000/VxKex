#include "buildcfg.h"
#include "kxbasep.h"

//
// If strong SharedUserData spoofing is enabled, this function
// supersedes KernelBase!GetSystemTimeAsFileTime because the original
// function reads system time from SharedUserData.
//
KXBASEAPI VOID WINAPI KxBasepGetSystemTimeAsFileTimeHook(
	OUT	PFILETIME	SystemTimeAsFileTime)
{
	ASSERT (KexData->IfeoParameters.StrongVersionSpoof & KEX_STRONGSPOOF_SHAREDUSERDATA);
	KexNtQuerySystemTime((PLONGLONG) SystemTimeAsFileTime);
}

//
// Same as above but this function supersedes GetSystemTime when doing
// SharedUserData-based version spoofing.
//
KXBASEAPI VOID WINAPI KxBasepGetSystemTimeHook(
	OUT	PSYSTEMTIME	SystemTime)
{
	LONGLONG Time;
	TIME_FIELDS TimeFields;

	ASSERT (KexData->IfeoParameters.StrongVersionSpoof & KEX_STRONGSPOOF_SHAREDUSERDATA);

	KexNtQuerySystemTime(&Time);
	RtlTimeToTimeFields(&Time, &TimeFields);

	//
	// Annoyingly, the TIME_FIELDS structure is not directly compatible with
	// the SYSTEMTIME structure...
	//

	SystemTime->wYear			= TimeFields.Year;
	SystemTime->wMonth			= TimeFields.Month;
	SystemTime->wDay			= TimeFields.Day;
	SystemTime->wDayOfWeek		= TimeFields.Weekday;
	SystemTime->wHour			= TimeFields.Hour;
	SystemTime->wMinute			= TimeFields.Minute;
	SystemTime->wSecond			= TimeFields.Second;
	SystemTime->wMilliseconds	= TimeFields.Milliseconds;
}

KXBASEAPI VOID WINAPI GetSystemTimePreciseAsFileTime(
	OUT	PFILETIME	SystemTimeAsFileTime)
{
	//
	// The real NtQuerySystemTime export from NTDLL is actually just a jump to
	// RtlQuerySystemTime, which reads from SharedUserData.
	//
	// However, if we are doing SharedUserData-based version spoofing, we will
	// overwrite that stub function with KexNtQuerySystemTime, so it is the best
	// of both worlds in terms of speed and actually working.
	//

	NtQuerySystemTime((PLONGLONG) SystemTimeAsFileTime);
}

KXBASEAPI BOOL WINAPI GetOverlappedResultEx( HANDLE file, OVERLAPPED *overlapped,
                                                     DWORD *result, DWORD timeout, BOOL alertable )
{
    NTSTATUS status;
    DWORD ret;

    /* Paired with the write-release in set_async_iosb() in ntdll; see the
     * latter for details. */
    status = (NTSTATUS)overlapped->Internal;
    if (status == STATUS_PENDING)
    {
        if (!timeout)
        {
            SetLastError( ERROR_IO_INCOMPLETE );
            return FALSE;
        }
        ret = WaitForSingleObjectEx( overlapped->hEvent ? overlapped->hEvent : file, timeout, alertable );
        if (ret == WAIT_FAILED)
            return FALSE;
        else if (ret)
        {
            SetLastError( ret );
            return FALSE;
        }

        /* We don't need to give this load acquire semantics; the wait above
         * already guarantees that the IOSB and output buffer are filled. */
        status = (NTSTATUS)overlapped->Internal;
        if (status == STATUS_PENDING) status = STATUS_SUCCESS;
    }

    *result = (DWORD)overlapped->InternalHigh;

	if (!NT_SUCCESS(status))
	{
		BaseSetLastNTError(status);
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

KXBASEAPI VOID WINAPI QueryUnbiasedInterruptTimePrecise(
	OUT	PULONGLONG	UnbiasedInterruptTimePrecise)
{
	QueryUnbiasedInterruptTime(UnbiasedInterruptTimePrecise);
}