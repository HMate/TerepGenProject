#if !defined(TEREPGEN_PLATFORM_H)

void TerminateGame();


typedef void *FileHandle;

FileHandle PlatformOpenFileForRead(char *FileName);
FileHandle PlatformOpenOrCreateFileForWrite(char *FileName);
uint32 PlatformReadFile(FileHandle Handle, void *Dest, uint32 Size);
uint32 PlatformWriteFile(FileHandle Handle, void *Source, uint32 Size);
void PlatformRenameFile(char* OldName, char* NewFileName);
bool32 FileIsEmpty(FileHandle Handle);

struct logger
{
    static void Print(char *Text);
    static void Print(char *Text, real64 Arg1);
    static void Print(char *Text, uint32 Arg1);
    static void PerfPrint(char *Text, real64 Arg1);
    static void PerfPrint(char *Text, real64 Arg1, uint32 Arg2);
    static void DebugPrint(char *Text);
    static void DebugPrint(char *Text, char *Arg1);
    static void DebugPrint(char *Text, uint32 Arg1);
    static void DebugPrint(char *Text, real32 Arg1);
    static void DebugPrint(char *Text, real32 Arg1, real32 Arg2);
    static void DebugPrint(char *Text, real64 Arg1);
};


struct avarage_time
{
    real64 AvgTime = 0.0f;
    real64 MeasureCount = 0.0f;
};

struct timer
{
    int64 Start;
    
    timer()
    {
        Reset();
    }
    
    void Reset();
    real64 GetSecondsElapsed();
    void PrintMiliSeconds(char *PrintText);
    void PrintSeconds(char *PrintText);
    void CalculateAverageTime(avarage_time *Avarage);
};
#define TEREPGEN_PLATFORM_H
#endif