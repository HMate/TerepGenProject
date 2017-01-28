
// NOTE: String utils

internal uint32
StringLength(char *Text)
{
    uint32 Length = 0;
    char *P = Text;
    while(*P != '\0')
    {
        Length++; P++;
    }
    return Length;
}

internal void
CopyString(char *Dest, char *Source)
{
    uint32 Index = 0;
    for(; Source[Index] != '\0'; Index++)
    {
        Dest[Index] = Source[Index];
    }
    Dest[Index] = Source[Index];
}

internal void
StringConcat(char* Result, char* Part0, char* Part1)
{
	uint32 i = 0, j = 0;
	while(Part0[i] != '\0')
	{
		Result[i] = Part0[i];
		i++;
	}
	
	while(Part1[j] != '\0')
	{
		Result[i] = Part1[j];
		i++; 
		j++;
	}
	Result[i] = '\0';
}

