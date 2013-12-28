//
//  tre.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/28/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

struct cmp_str
{
    bool operator()(const char*a, const char*b)
    {
        return strcmp(a, b) < 0;
    }
} ;

typedef struct TreEntry{
    
    uint8_t     unknownFlag;
    char        name[65];
    size_t      size;
    uint8_t*    data;
    
} TreEntry;



class TreArchive{
    
public:
    
     TreArchive();
    ~TreArchive();
 
    bool InitFromFile(const char* filepath);
    void InitFromRAM(uint8_t* data, size_t size);
    
    char* GetPath(void);
    
    void List(FILE* output);
    
    //Direct access to a TRE entry.
    TreEntry* GetEntryByName(const char* entryName);
    
    //A way to iterate through all entries in the TRE.
    TreEntry* GetEntryByID(size_t entryID);
    size_t GetNumEntries(void);
    
    bool Decompress(const char* dstDirectory);
    
private:
    
    
    std::vector<TreEntry*> orderedEntries;
    
    void ReadEntry(ByteStream* stream, TreEntry* entry);
    void Parse(void);
    
    //
    char path[512];
    uint8_t* data;
    size_t   size;
    
    
    
    // allows to know if we should free the TRE data
    bool initalizedFromFile ;
    

    std::map<const char*,TreEntry*,cmp_str> entries;
};