#include "CvGameCoreDLL.h"

#include "CvGameCoreDLLUndefNew.h"

#include <new>

#include "CvGlobals.h"
#include "FProfiler.h"
#include "CvDLLInterfaceIFaceBase.h"

#include <psapi.h>

#ifdef MEMORY_TRACKING
void	ProfileTrackAlloc(void* ptr, int size);

void	ProfileTrackDeAlloc(void* ptr);

void DumpMemUsage(const char* fn, int line)
{
	PROCESS_MEMORY_COUNTERS pmc;

	if ( GetProcessMemoryInfo( GetCurrentProcess(), &pmc, sizeof(pmc)) )
	{
		char buffer[200];

		sprintf(buffer, "memory (%s,%d): %d Kbytes, peak %d\n", fn, line, (int)(pmc.WorkingSetSize/1024), (int)(pmc.PeakWorkingSetSize/1024));
		OutputDebugString(buffer);
	}
}

#define	PROFILE_TRACK_ALLOC(x,n)	ProfileTrackAlloc(x,n);
#define	PROFILE_TRACK_DEALLOC(x)	ProfileTrackDeAlloc(x);
#else
#define	PROFILE_TRACK_ALLOC(x,n)	
#define	PROFILE_TRACK_DEALLOC(x)	
#endif

static allocCount = 0;

//
// operator global new and delete override for gamecore DLL 
//
void *__cdecl operator new(size_t size)
{
	if (gDLL)
	{
		return gDLL->newMem(size, __FILE__, __LINE__);
	}
	return malloc(size);
}

void __cdecl operator delete (void *p)
{
	if (gDLL)
	{
		gDLL->delMem(p, __FILE__, __LINE__);
	}
	else
	{
		free(p);
	}
}

void* operator new[](size_t size)
{
	if (gDLL)
		return gDLL->newMemArray(size, __FILE__, __LINE__);
	return malloc(size);
}

void operator delete[](void* pvMem)
{
	if (gDLL)
	{
		gDLL->delMemArray(pvMem, __FILE__, __LINE__);
	}
	else
	{
		free(pvMem);
	}
}

void *__cdecl operator new(size_t size, char* pcFile, int iLine)
{
	return gDLL->newMem(size, pcFile, iLine);
}

void *__cdecl operator new[](size_t size, char* pcFile, int iLine)
{
	return gDLL->newMem(size, pcFile, iLine);
}

void __cdecl operator delete(void* pvMem, char* pcFile, int iLine)
{
	gDLL->delMem(pvMem, pcFile, iLine);
}

void __cdecl operator delete[](void* pvMem, char* pcFile, int iLine)
{
	gDLL->delMem(pvMem, pcFile, iLine);
}


void* reallocMem(void* a, unsigned int uiBytes, const char* pcFile, int iLine)
{
	return gDLL->reallocMem(a, uiBytes, pcFile, iLine);
}

unsigned int memSize(void* a)
{
	return gDLL->memSize(a);
}

// BUG - EXE/DLL Paths - start
HANDLE dllModule = NULL;
// BUG - EXE/DLL Paths - end

BOOL APIENTRY DllMain(HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved)
{
	switch( ul_reason_for_call ) {
	case DLL_PROCESS_ATTACH:
		{

// BUG - EXE/DLL Paths - start
		dllModule = hModule;
// BUG - EXE/DLL Paths - end

		// The DLL is being loaded into the virtual address space of the current process as a result of the process starting up 
		OutputDebugString("DLL_PROCESS_ATTACH\n");

		// set timer precision
		MMRESULT iTimeSet = timeBeginPeriod(1);		// set timeGetTime and sleep resolution to 1 ms, otherwise it's 10-16ms
		FAssertMsg(iTimeSet==TIMERR_NOERROR, "failed setting timer resolution to 1 ms");
		}
		break;
	case DLL_THREAD_ATTACH:
		// OutputDebugString("DLL_THREAD_ATTACH\n");
		break;
	case DLL_THREAD_DETACH:
		// OutputDebugString("DLL_THREAD_DETACH\n");
		break;
	case DLL_PROCESS_DETACH:

// BUG - EXE/DLL Paths - start
		dllModule = NULL;
// BUG - EXE/DLL Paths - end

		OutputDebugString("DLL_PROCESS_DETACH\n");
		timeEndPeriod(1);
		GC.setDLLIFace(NULL);
		break;
	}
	
	return TRUE;	// success
}

#ifdef USE_INTERNAL_PROFILER

//	Uncomment the following line to provide (very) detailed tracing to
//	the debug stream (view with DbgView or a debugger)
#define DETAILED_TRACE

#define MAX_SAMPLES 900
static ProfileSample* _currentSample = NULL;
static int numSamples = 0;
static ProfileSample* sampleList[MAX_SAMPLES];
static ProfileSample* sampleStack[MAX_SAMPLES];
static int depth = -1;

#ifdef DETAILED_TRACE
static ProfileSample* lastExit = NULL;
static int exitCount = 0;
static bool detailedTraceEnabled = false;

static void GenerateTabString(char* buffer,int n)
{
	while(n-- > 0)
	{
		*buffer++ = '\t';
	}

	*buffer = '\0';
}
#endif

void EnableDetailedTrace(bool enable)
{
#ifdef DETAILED_TRACE
	detailedTraceEnabled = enable;
#endif
}

void IFPBeginSample(ProfileSample* sample)
{
	if ( sample->Id == -1 )
	{
		if ( numSamples == MAX_SAMPLES )
		{
			dumpProfileStack();
			::MessageBox(NULL,"Profile sample limit exceeded","CvGameCore",MB_OK);
			return;
		}
		sample->Id = numSamples;
		sample->OpenProfiles = 0;
		sample->ProfileInstances = 0;
		sample->Accumulator.QuadPart = 0;
		sample->ChildrenSampleTime.QuadPart = 0;
		sampleList[numSamples++] = sample;
	}

	if ( ++depth == MAX_SAMPLES )
	{
		::MessageBox(NULL,"Sample stack overflow","CvGameCore",MB_OK);
	}
	else
	{
		sampleStack[depth] = sample;
	}

	sample->ProfileInstances++;
	sample->OpenProfiles++;

	if ( sample->OpenProfiles == 1 )
	{
		if ( _currentSample == NULL )
		{
			sample->Parent = -1;
		}
		else
		{
			sample->Parent = _currentSample->Id;
		}

		QueryPerformanceCounter(&sample->StartTime);
	}

	_currentSample = sample;

#ifdef DETAILED_TRACE
	if ( detailedTraceEnabled && lastExit != sample )
	{
		char buffer[300];

		if ( exitCount != 0 )
		{
			GenerateTabString(buffer, depth);
			sprintf(buffer+depth, "[%d]\n", exitCount);
			OutputDebugString(buffer);

			exitCount = 0;
		}

		GenerateTabString(buffer, depth);
		sprintf(buffer+depth, "-->%s\n", sample->Name);

		OutputDebugString(buffer);
	}
#endif
}

void IFPEndSample(ProfileSample* sample)
{
	if ( _currentSample != sample && sample->Parent != -1 )
	{
		MessageBox(NULL,"Sample closure not matched","CvGameCore",MB_OK);
	}

	if ( depth < 0 )
	{
		if ( sample->Parent != -1 )
		{
			MessageBox(NULL,"Too many end-samples","CvGameCore",MB_OK);
		}
	}
	else 
	{
		if ( depth == 0 )
		{
			_currentSample = NULL;
			depth = -1;
		}
		else
		{
			_currentSample = sampleStack[--depth];
		}

		if ( sample->OpenProfiles-- == 1 )
		{
			LARGE_INTEGER now;
			LONGLONG ellapsed;

			QueryPerformanceCounter(&now);

			ellapsed = (now.QuadPart - sample->StartTime.QuadPart);
			sample->Accumulator.QuadPart += ellapsed;

			if ( _currentSample != NULL )
			{
				_currentSample->ChildrenSampleTime.QuadPart += ellapsed;
			}
		}

#ifdef DETAILED_TRACE
		if ( detailedTraceEnabled && lastExit != sample )
		{
			char buffer[300];

			GenerateTabString(buffer, depth+1);
			strcpy(buffer+depth+1, "...\n");

			OutputDebugString(buffer);
			exitCount = 1;
		}
		else
		{
			exitCount++;
		}

		lastExit = sample;
#endif
	}
}

void IFPBegin(void)
{
	for(int i = 0; i < numSamples; i++ )
	{
		sampleList[i]->Accumulator.QuadPart = 0;
		sampleList[i]->ChildrenSampleTime.QuadPart = 0;
		sampleList[i]->ProfileInstances = sampleList[i]->OpenProfiles;
	}
}

void IFPEnd(void)
{
	//	Log the timings
	char buffer[300];
	LARGE_INTEGER freq;

	QueryPerformanceFrequency(&freq);

	// gvd g_DLL->logMsg("IFP_log.txt","Fn\tTime (mS)\tAvg time\t#calls\tChild time\tSelf time\tParent\n");

	for(int i = 0; i < numSamples; i++ )
	{
		if ( sampleList[i]->ProfileInstances != 0 )
		{
			sprintf(buffer,
					"%s\t%d\t%d\t%d\t%d\t%d\t%s\n",
					sampleList[i]->Name,
					(int)((1000*sampleList[i]->Accumulator.QuadPart)/freq.QuadPart),
					(int)((1000*sampleList[i]->Accumulator.QuadPart)/(freq.QuadPart*sampleList[i]->ProfileInstances)),
					sampleList[i]->ProfileInstances,
					(int)((1000*sampleList[i]->ChildrenSampleTime.QuadPart)/freq.QuadPart),
					(int)((1000*sampleList[i]->Accumulator.QuadPart)/freq.QuadPart) - (int)((1000*sampleList[i]->ChildrenSampleTime.QuadPart)/freq.QuadPart),
					sampleList[i]->Parent == -1 ? "" : sampleList[sampleList[i]->Parent]->Name);
			// gvd g_DLL->logMsg("IFP_log.txt",buffer);
		}
	}
}

static bool isInLongLivedSection = false;
static ProfileSample rootSample__("Root");

//
// dump the current (profile) call stack to debug output
//
void dumpProfileStack(void)
{
	int i = 0;
	int dumpDepth = depth;
	char buffer[200];

	OutputDebugString("Profile stack:\n");

	while(dumpDepth >= 0)
	{
		char* ptr = buffer;

		i++;
		for(int j = 0; j < i; j++)
		{
			*ptr++ = '\t';
		}
		strcpy(ptr,sampleStack[dumpDepth--]->Name);
		strcat(ptr, "\n");
		OutputDebugString(buffer);
	}
}

static int pythonDepth = 0;

bool IFPPythonCall(const char* callerFn, const char* moduleName, const char* fxnName, void* fxnArg)
{
	PROFILE("IFPPythonCall1");

	//OutputDebugString(CvString::format("Python call to %s::%s [%d]\n", moduleName, fxnName, pythonDepth++).c_str());

	bool result = gDLL->getPythonIFace()->callFunction(moduleName, fxnName, fxnArg);
	
	//OutputDebugString("...complete\n");
	pythonDepth--;

	return result;
}

bool IFPPythonCall(const char* callerFn, const char* moduleName, const char* fxnName, void* fxnArg, long* result)
{
	PROFILE("IFPPythonCall2");

	//OutputDebugString(CvString::format("Python call to %s::%s [%d]\n", moduleName, fxnName, pythonDepth++).c_str());

	bool bResult = gDLL->getPythonIFace()->callFunction(moduleName, fxnName, fxnArg, result);
	
	//OutputDebugString("...complete\n");
	pythonDepth--;

	return bResult;
}


bool IFPPythonCall(const char* callerFn, const char* moduleName, const char* fxnName, void* fxnArg, CvString* result)
{
	PROFILE("IFPPythonCall3");

	//OutputDebugString(CvString::format("Python call to %s::%s [%d]\n", moduleName, fxnName, pythonDepth++).c_str());

	bool bResult = gDLL->getPythonIFace()->callFunction(moduleName, fxnName, fxnArg, result);
	
	//OutputDebugString("...complete\n");
	pythonDepth--;

	return bResult;
}


bool IFPPythonCall(const char* callerFn, const char* moduleName, const char* fxnName, void* fxnArg, CvWString* result)
{
	PROFILE("IFPPythonCall4");

	//OutputDebugString(CvString::format("Python call to %s::%s [%d]\n", moduleName, fxnName, pythonDepth++).c_str());

	bool bResult = gDLL->getPythonIFace()->callFunction(moduleName, fxnName, fxnArg, result);
	
	//OutputDebugString("...complete\n");
	pythonDepth--;

	return bResult;
}


bool IFPPythonCall(const char* callerFn, const char* moduleName, const char* fxnName, void* fxnArg, std::vector<byte>* pList)
{
	PROFILE("IFPPythonCall5");

	//OutputDebugString(CvString::format("Python call to %s::%s [%d]\n", moduleName, fxnName, pythonDepth++).c_str());

	bool result = gDLL->getPythonIFace()->callFunction(moduleName, fxnName, fxnArg, pList);
	
	//OutputDebugString("...complete\n");
	pythonDepth--;

	return result;
}


bool IFPPythonCall(const char* callerFn, const char* moduleName, const char* fxnName, void* fxnArg, std::vector<int> *pIntList)
{
	PROFILE("IFPPythonCall6");

	//OutputDebugString(CvString::format("Python call to %s::%s [%d]\n", moduleName, fxnName, pythonDepth++).c_str());

	bool result = gDLL->getPythonIFace()->callFunction(moduleName, fxnName, fxnArg, pIntList);
	
	//OutputDebugString("...complete\n");
	pythonDepth--;

	return result;
}


bool IFPPythonCall(const char* callerFn, const char* moduleName, const char* fxnName, void* fxnArg, int* pIntList, int* iListSize)
{
	PROFILE("IFPPythonCall7");

	//OutputDebugString(CvString::format("Python call to %s::%s [%d]\n", moduleName, fxnName, pythonDepth++).c_str());

	bool result = gDLL->getPythonIFace()->callFunction(moduleName, fxnName, fxnArg, pIntList, iListSize);
	
	//OutputDebugString("...complete\n");
	pythonDepth--;

	return result;
}


bool IFPPythonCall(const char* callerFn, const char* moduleName, const char* fxnName, void* fxnArg, std::vector<float> *pFloatList)
{
	PROFILE("IFPPythonCall8");

	//OutputDebugString(CvString::format("Python call to %s::%s [%d]\n", moduleName, fxnName, pythonDepth++).c_str());

	bool result = gDLL->getPythonIFace()->callFunction(moduleName, fxnName, fxnArg, pFloatList);
	
	//OutputDebugString("...complete\n");
	pythonDepth--;

	return result;
}



#endif

//
// enable dll profiler if necessary, clear history
//
void startProfilingDLL(bool longLived)
{
#ifdef USE_INTERNAL_PROFILER
	if ( longLived )
	{
		isInLongLivedSection = true;
	}
	else if (GC.isDLLProfilerEnabled() && !isInLongLivedSection)
	{
		IFPBegin();
		IFPBeginSample(&rootSample__);
	}
#else
	if (GC.isDLLProfilerEnabled())
	{
		gDLL->ProfilerBegin();
	}
#endif
}

//
// dump profile stats on-screen
//
void stopProfilingDLL(bool longLived)
{
#ifdef USE_INTERNAL_PROFILER
	if ( longLived )
	{
		isInLongLivedSection = false;
	}
	else if (GC.isDLLProfilerEnabled() && !isInLongLivedSection)
	{
		IFPEndSample(&rootSample__);
		IFPEnd();
	}
#else
	if (GC.isDLLProfilerEnabled())
	{
		gDLL->ProfilerEnd();
	}
#endif
}

#ifdef MEMORY_TRACKING
CMemoryTrack*	CMemoryTrack::trackStack[MAX_TRACK_DEPTH];
int CMemoryTrack::m_trackStackDepth = 0;

CMemoryTrack::CMemoryTrack(const char* name, bool valid)
{
	m_highWater = 0;
	m_seq = 0;
	m_name = name;
	m_valid = valid;

	if ( m_trackStackDepth < MAX_TRACK_DEPTH )
	{
		trackStack[m_trackStackDepth++] = this;
	}
}

CMemoryTrack::~CMemoryTrack()
{
	if ( m_valid )
	{
		for(int i = 0; i < m_highWater; i++)
		{
			if ( m_track[i] != NULL )
			{
				char buffer[200];

				sprintf(buffer, "Apparent memory leak (size %d) detected in %s (from %s:%d)\n", m_allocSize[i], m_name, m_trackName[i], m_allocSeq[i]);
				OutputDebugString(buffer);
			}
		}
	}

	if ( trackStack[m_trackStackDepth-1] == this )
	{
		m_trackStackDepth--;
	}
}

void CMemoryTrack::NoteAlloc(void* ptr, int size)
{
	if ( m_valid )
	{
		for(int i = 0; i < m_highWater; i++)
		{
			if ( m_track[i] == NULL )
			{
				break;
			}
		}

		if ( i == m_highWater )
		{
			if ( m_highWater < MAX_TRACKED_ALLOCS )
			{
				m_highWater++;
			}
			else
			{
				m_valid = false;
				return;
			}
		}

		m_track[i] = ptr;
		m_allocSeq[i] = m_seq++;
		m_allocSize[i] = size;
#ifdef USE_INTERNAL_PROFILER
		m_trackName[i] = _currentSample == NULL ? "<None>" : _currentSample->Name;
#else
		m_trackName[i] = "";
#endif
	}
}

void CMemoryTrack::NoteDeAlloc(void* ptr)
{
	if ( m_valid )
	{
		for(int i = 0; i < m_highWater; i++)
		{
			if ( m_track[i] == ptr )
			{
				m_track[i] = NULL;
				break;
			}
		}
	}
}

CMemoryTrack* CMemoryTrack::GetCurrent(void)
{
	if ( 0 < m_trackStackDepth && m_trackStackDepth < MAX_TRACK_DEPTH )
	{
		return trackStack[m_trackStackDepth-1];
	}
	else
	{
		return NULL;
	}
}

CMemoryTrace::CMemoryTrace(const char* name)
{
	PROCESS_MEMORY_COUNTERS pmc;

	GetProcessMemoryInfo( GetCurrentProcess(), &pmc, sizeof(pmc));
	m_name = name;
	m_start = pmc.WorkingSetSize;
}

CMemoryTrace::~CMemoryTrace()
{
	static PROCESS_MEMORY_COUNTERS lastPmc;
	PROCESS_MEMORY_COUNTERS pmc;

	if ( GetProcessMemoryInfo( GetCurrentProcess(), &pmc, sizeof(pmc)) && (pmc.WorkingSetSize != m_start || lastPmc.WorkingSetSize/1024 != pmc.WorkingSetSize/1024))
	{
		char buffer[200];

		sprintf(buffer, "function %s added %dK bytes, total now %uK\n", m_name, ((int)(pmc.WorkingSetSize - m_start))/1024, (unsigned int)pmc.WorkingSetSize/1024);
		OutputDebugString(buffer);
	}

	memcpy(&lastPmc, &pmc, sizeof(pmc));
}

void ProfileTrackAlloc(void* ptr, int size)
{
	CMemoryTrack* current = CMemoryTrack::GetCurrent();

	if ( current != NULL )
	{
		current->NoteAlloc(ptr, size);
	}
}

void ProfileTrackDeAlloc(void* ptr)
{
	CMemoryTrack* current = CMemoryTrack::GetCurrent();

	if ( current != NULL )
	{
		current->NoteDeAlloc(ptr);
	}
}

#endif