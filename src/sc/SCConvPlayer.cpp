//
//  SCConvPlayer.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


SCConvPlayer::SCConvPlayer():
    conversationID(0),
    initialized(false)
{
    
}

SCConvPlayer::~SCConvPlayer(){
    
}

#define GROUP_SHOT              0x00
#define GROUP_SHOT_ADD_CHARCTER 0x01
#define GROUP_SHOT_CHARCTR_TALK 0x02
#define CLOSEUP                 0x03
#define CLOSEUP_CONTINUATION    0x04
#define SHOW_TEXT               0x0A
#define YESNOCHOICE_BRANCH1     0x0B
#define YESNOCHOICE_BRANCH2     0x0C
#define UNKNOWN                 0x0E
#define CHOOSE_WINGMAN          0x0F

void SCConvPlayer::Focus(void) {
    IActivity::Focus();
    //printf("Conv Player running Frame on convID=%d.\n",this->conversationID);
}

void SCConvPlayer::ReadNextFrame(void){
    
    
    
    
        
        if (read == size){
            Stop();
            return;
        }

        
        currentFrame.creationTime = SDL_GetTicks();
        
        uint8_t* startPos = conv.GetPosition();
    
        uint8_t type = conv.ReadByte();
    
        switch (type) {
        case GROUP_SHOT:  // Group plan
        {
            char* location = (char*)conv.GetPosition();
            
            ConvBackGround* bg = ConvAssets.GetBackGround(location);
            
            
            currentFrame.mode = ConvFrame::CONV_WIDE;
            currentFrame.participants.clear();
            currentFrame.bgLayers = &bg->layers;
            currentFrame.bgPalettes = &bg->palettes;
            
            printf("ConvID: %d WIDEPLAN : LOCATION: '%s'\n",this->conversationID, location);
            conv.MoveForward(8+1);
            
            while(conv.PeekByte() == GROUP_SHOT_ADD_CHARCTER)
                ReadNextFrame();
            
            break;
        }
        case CLOSEUP:  // Person talking
        {
            char* speakerName =       (char*)conv.GetPosition();
            char* setName         = (char*)conv.GetPosition() + 0xA;
            char* sentence         = (char*)conv.GetPosition() + 0x17;
            
            
            currentFrame.mode = ConvFrame::CONV_CLOSEUP;
            currentFrame.participants.clear();
            NPCChar* participant = ConvAssets.GetPNCChar(speakerName);
            currentFrame.participants.push_back(participant);
            
            ConvBackGround* bg = ConvAssets.GetBackGround(setName);
            currentFrame.bgLayers = &bg->layers;
            currentFrame.bgPalettes = &bg->palettes;
            
            
            conv.MoveForward(0x17 + strlen((char*)sentence)+1);
            uint8_t color = conv.ReadByte(); // Color ?
            currentFrame.textColor = color;
            
            printf("ConvID: %d CLOSEUP: WHO: '%8s' WHERE: '%8s'     WHAT: '%s' (%2X)\n",this->conversationID,speakerName,setName,sentence,color);
            break;
        }
        case CLOSEUP_CONTINUATION:  // Same person keep talking
        {
            char* sentence         = (char*)conv.GetPosition();
            
            currentFrame.text = sentence;
            
            conv.MoveForward(strlen((char*)sentence)+1);
            printf("ConvID: %d MORETEX:                                       WHAT: '%s'\n",this->conversationID,sentence);
            break;
        }
        case YESNOCHOICE_BRANCH1:  // Choice Offsets are question
        {
            currentFrame.mode = ConvFrame::CONV_CONTRACT_CHOICE;
            printf("ConvID: %d CHOICE YES/NO : %X.\n",this->conversationID,type);
            //Looks like first byte is the offset to skip if the answer is no.
            /*uint8_t noOffset  =*/  conv.ReadByte();
            /*uint8_t yesOffset  =*/ conv.ReadByte();
            break;
        }
        case YESNOCHOICE_BRANCH2:  // Choice offset after first branch
        {
            
            //currentFrame.mode = ConvFrame::CONV_CONTRACT_CHOICE;
            printf("ConvID: %d CHOICE YES/NO : %X.\n",this->conversationID,type);
            //Looks like first byte is the offset to skip if the answer is no.
            /*uint8_t yesOffset  =*/ conv.ReadByte();
            /*uint8_t noOffset  =*/  conv.ReadByte();
            break;
        }
        case GROUP_SHOT_ADD_CHARCTER:  // Add person to GROUP
        {
            
            char* participantName  = (char*)conv.GetPosition();
            NPCChar* participant = ConvAssets.GetPNCChar(participantName);
            currentFrame.participants.push_back(participant);
            
            printf("ConvID: %d WIDEPLAN ADD PARTICIPANT: '%s'\n",this->conversationID,conv.GetPosition());
            conv.MoveForward(0xD);
            break;
        }
        case GROUP_SHOT_CHARCTR_TALK:  // Make group character talk
        {
            
            char* who = (char*)conv.GetPosition();
            conv.MoveForward(0xE);
            char* sentence = (char*)conv.GetPosition();
            conv.MoveForward(strlen(sentence)+1);
            printf("ConvID: %d WIDEPLAN PARTICIPANT TALKING: who: '%s' WHAT '%s'\n",this->conversationID,who,sentence);
            
            
            break;
        }
        case SHOW_TEXT:  // Show text
        {
            int8_t color = conv.ReadByte();
            char* sentence = (char*)conv.GetPosition();
            
            currentFrame.text = sentence;
            currentFrame.textColor = color;
            
             printf("ConvID: %d Show Text: '%s' \n",this->conversationID,sentence);
            conv.MoveForward(strlen(sentence)+1);
            
            break;
        }
        case 0xE:
        {
            uint8_t unkn  = conv.ReadByte();
            uint8_t unkn1  = conv.ReadByte();
            printf("ConvID: %d Unknown usage Flag 0xE: (0x%2X 0x%2X) \n",this->conversationID,unkn,unkn1);
            break;
        }
        case CHOOSE_WINGMAN:  // Wingman selection trigger
        {
            currentFrame.mode = ConvFrame::CONV_WINGMAN_CHOICE;
            printf("ConvID: %d Open pilot selection screen with current BG.\n",this->conversationID);
            break;
        }
        default:
            printf("ConvID: %d Unknown opcode: %X.\n",this->conversationID,type);
            Stop();
            return ;
            break;
        }
    
    
    read += conv.GetPosition() - startPos;
    this->currentFrame.SetExpired(false);
}

void SCConvPlayer::SetArchive(PakEntry* convPakEntry){
    
    if (convPakEntry->size == 0){
        Game.Log("Conversation entry is empty: Unable to load it.\n");
        Stop();
        return;
    }
    
    this->size = convPakEntry->size;
    
    this->conv.Set(convPakEntry->data);
    
    //Read a frame so we are ready to display it.
    //ReadNextFrame();
    this->currentFrame.SetExpired(true);
    
    initialized = true;
}


void SCConvPlayer::SetID(int32_t id){
    
    this->conversationID = id;
    
    TreEntry* convEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\CONV.PAK");
    
    PakArchive convPak;
    convPak.InitFromRAM("CONV.PAK", convEntry->data, convEntry->size);
    //convPak.List(stdout);
    
    if (convPak.GetNumEntries() <= id){
        Stop();
        Game.Log("Cannot load conversation id (max convID is %lu).",convPak.GetNumEntries()-1);
        return;
    }
    
    SetArchive(convPak.GetEntry(id));
    
}

void SCConvPlayer::Init( ){
    VGAPalette* rendererPalette = VGA.GetPalette();
    this->palette = *rendererPalette;
}

void SCConvPlayer::CheckFrameExpired(void){
    
    //A frame expires either after a player press a key, click or 6 seconds elapse.
    //Mouse
    SDL_Event mouseEvents[5];
    int numMouseEvents= SDL_PeepEvents(mouseEvents,5,SDL_PEEKEVENT,SDL_MOUSEBUTTONUP,SDL_MOUSEBUTTONUP);
    for(int i= 0 ; i < numMouseEvents ; i++){
        SDL_Event* event = &mouseEvents[i];
        
        switch (event->type) {
            case SDL_MOUSEBUTTONUP:
                this->currentFrame.SetExpired(true);
                break;
            default:
                break;
        }
    }
    
    
    //Keyboard
    SDL_Event keybEvents[5];
    int numKeybEvents = SDL_PeepEvents(keybEvents,5,SDL_PEEKEVENT,SDL_KEYUP,SDL_KEYUP);
    for(int i= 0 ; i < numKeybEvents ; i++){
        SDL_Event* event = &keybEvents[i];
        switch (event->type) {
            default:
                this->currentFrame.SetExpired(true);
                break;
        }
    }
    
    /*
    int32_t currentTime = SDL_GetTicks();
    if(currentTime - currentFrame.creationTime > 5000)
        this->currentFrame.SetExpired(true);
    */
    
    
   
}


void SCConvPlayer::DrawText(const char* text, uint8_t type){
    
}



void SCConvPlayer::RunFrame(void){
    
    
    if (!initialized){
        Stop();
        Game.Log("Conv ID %d was not initialized: Stopping.\n", this->conversationID);
        return ;
    }
    
    //If frame needs to be update
    CheckFrameExpired();
    if ( currentFrame.IsExpired() )
        ReadNextFrame();
    
    
    CheckButtons();
    
    VGA.Activate();
    VGA.Clear();
    
    //Update the palette for the current background
    for (size_t i = 0; i < currentFrame.bgLayers->size(); i++) {
        ByteStream paletteReader;
        paletteReader.Set((*currentFrame.bgPalettes)[i]);
        this->palette.ReadPatch(&paletteReader);
        VGA.SetPalette(&this->palette);

    }
    
    
    //Draw static
    for (size_t i = 0; i < currentFrame.bgLayers->size(); i++) {
        VGA.DrawShape((*currentFrame.bgLayers)[i]);
    }

    
    
    for (size_t i=0 ; i < currentFrame.participants.size(); i++) {
        NPCChar* participant = currentFrame.participants[i];
  //      VGA.DrawShape(participant->appearance);
    }
    
    //Draw text
    DrawText(currentFrame.text, currentFrame.textColor);
    
    DrawButtons();
    
    
    if (currentFrame.mode == ConvFrame::CONV_WIDE ||
        currentFrame.mode == ConvFrame::CONV_CLOSEUP)
        ;
    
    //Draw Mouse
    Mouse.Draw();
    
    //Check Mouse state.
    
    VGA.VSync();
    
    
}
