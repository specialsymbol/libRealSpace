//
//  PakArchive.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 12/29/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


PakArchive::PakArchive(){
    
}

PakArchive::~PakArchive(){
    if (initalizedFromFile)
        delete[] this->data;
}



void PakArchive::Parse(void){
    
    uint32_t advertisedSize = stream.ReadUInt32LE();
    
    if (advertisedSize != this->size){
        printf("'%s' is not a PAK archive !.\n",this->path);
        return;
    }
    
    //Parse rest
    ByteStream peek(this->stream);
    uint32_t offset = peek.ReadUInt32LE();
    offset &= 0x00FFFFFF ; //Remove the leading 0xE0
    
    uint32_t numEntries = (offset-4)/4;
    
    for(int i =0 ; i < numEntries ; i ++){
        
        PakEntry* entry = new PakEntry();
        
        offset = stream.ReadUInt32LE();
        offset &= 0x00FFFFFF ; //Remove the leading 0xE0
        
        entry->data = this->data + offset;
        
        entries.push_back(entry);
    }
    
    //Second pass to calculate the sizes.
    int i =0;
    for( ; i < numEntries-1 ; i ++){
        
        PakEntry* entry = entries[i];
        
        entry->size = entries[i+1]->data - entry->data;
    }
    
    PakEntry* entry = entries[i];
    entry->size = this->data+this->size - entries[i-1]->data;
    
}

bool PakArchive::InitFromFile(const char* filepath){
    char fullPath[512] ;
    fullPath[0] = '\0';
    
    strcat(fullPath, GetBase());
    strcat(fullPath, filepath);
    
    
    FILE* file = fopen(fullPath, "r");
    
    if (!file){
        printf("Unable to open TRE archive: '%s'.\n",filepath);
        return false;
    }
    
    fseek(file, 0,SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file,0 ,SEEK_SET);
    
    uint8_t* fileData = new uint8_t[fileSize];
    fread(fileData, 1, fileSize, file);
    
    strcpy(this->path, filepath);
    
    InitFromRAM(fileData,fileSize);
    
    
    fclose(file);
    
    return true;

}

void PakArchive::InitFromRAM(uint8_t* data, size_t size){
    
    if (this->path[0] == '\0')
        strcpy(this->path, "From RAM");
    
    this->data = data;
    this->size = size;
    
    stream.Set(this->data);
    
    Parse();

}

bool PakArchive::Decompress(const char* dstDirectory){
    return true;
}

size_t PakArchive::GetNumEntries(void){
    return this->entries.size();
}

PakEntry* PakArchive::GetEntry(size_t index){
    return this->entries[index];
}

void PakArchive::List(FILE* output){
    fprintf(output,"Listing content of PAK archives '%s'\n",this->path);
    for(size_t i =0; i < GetNumEntries() ; i++){
        PakEntry* entry = entries[i];
        if (entry->size != 0)
            fprintf(output,"    Entry [%3lu] size: %7lu bytes.\n",i, entry->size);
        else
            fprintf(output,"    Entry [%3lu] size: %7lu bytes (DUPLICATE).\n",i, entry->size);
    }
}

void PakArchive::GuessContent(FILE* output){
    
}