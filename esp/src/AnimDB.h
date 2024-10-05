#pragma once

#include <Preferences.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <unistd.h>
#include "Debug.h"
#include "FileManager.h"

#define PREF_NAME "AnimDB"
#define ANIM_NUM_KEY "animNum"
#define ANIM_ARR_KEY "anim_arr"

#define ID_MAX_DIGITS 2
#define ID_MAX_VALUE 99 

// FILE PATH: ANIM_PATH_PREFIX + {Animation.id} + ANIM_PATH_SUFFIX
#define ANIM_PATH_PREFIX "/anim"
#define ANIM_PATH_SUFFIX ".dat"

// NAME KEY: ANIM_NAME_KEY + {Animation.id}
#define ANIM_NAME_KEY "anim_name"
#define MAX_KEY_SIZE 15

struct Animation{
    unsigned short id;
    unsigned int length;
};

class AnimDB{
public: 
    AnimDB();
    ~AnimDB();

    void print();
    void clear();

    boolean addAnimation(const char* name, const byte* data, size_t length); 
    size_t getAnimationSize(const char* name);
    boolean getAnimation(const char* name, byte* data, size_t length);
    boolean removeAnimation(const char* name);
    boolean getAllAnimationNames(char* buffer, size_t bufferLength);

    SizeInfo getSizeInfo();

private:
    Preferences preferences;
    boolean flag = true;
    Animation* animArr;
    char* names = NULL;
    size_t namesLen = 0;

    String tempName;
    char tempKey[MAX_KEY_SIZE + ID_MAX_DIGITS];

    boolean addAnimationName(unsigned short id, const char* name);
    boolean addAnimationData(unsigned short id, const byte* data, size_t length);
    boolean addIdToKey(unsigned short id);

    unsigned short getAnimationId(const char* name,unsigned short animNum);
    unsigned short getNewId(unsigned short animNum);
    boolean getAnimationData(unsigned short id, byte* data, size_t length);
    boolean removeAnimationData(unsigned short id);
    boolean removeAnimationName(unsigned short id);

    //writing result to this->tempKey
    boolean createKey(const char* pref);
    boolean createKey(const char* pref, unsigned short id);
    boolean createKey(const char* pref, unsigned short id, const char* suff);

    unsigned short prepareAnimArr(boolean extraSpace = false);
    bool saveAnimArr(unsigned short animNum);
    void setNames(char* buffer, size_t len);
    void resetNames();
};

AnimDB::AnimDB(){
    FileManager::begin();
    preferences.begin(PREF_NAME);

    if(!preferences.isKey(ANIM_NUM_KEY)){
        Debug::info("Animation number key not set - initialising AnimDB\n");
        preferences.clear();
        size_t size = preferences.putUShort(ANIM_NUM_KEY, 0);
        if(size == 0){
            Debug::error("Failed to initialise AnimDB\n");
        }
    }

    animArr=NULL;
    
    print();
}

unsigned short AnimDB::prepareAnimArr(boolean extraSpace){
    unsigned short animNum;
    if(!preferences.isKey(ANIM_NUM_KEY) || !preferences.isKey(ANIM_ARR_KEY)){
        animNum = 0;
        preferences.putUShort(ANIM_NUM_KEY, animNum);
        if(animArr != NULL)free(animArr);
        animArr = (Animation*)malloc(sizeof(Animation));
        return animNum;
    }
    animNum = preferences.getUShort(ANIM_NUM_KEY);
    if(animArr != NULL && !extraSpace){
        return animNum = preferences.getUShort(ANIM_NUM_KEY);
    }
    if(animArr!=NULL)free(animArr);
    size_t animArrSize = sizeof(Animation) * animNum;
    animArr = (Animation*)malloc(animArrSize + sizeof(Animation));
    size_t err = preferences.getBytes(ANIM_ARR_KEY, animArr, animArrSize);

    return animNum;
}

boolean AnimDB::saveAnimArr(unsigned short animNum){
    preferences.putBytes(ANIM_ARR_KEY, animArr, sizeof(Animation) * animNum);
    preferences.putUShort(ANIM_NUM_KEY, animNum);
    free(animArr);
    animArr = NULL;
    return true;
}

void AnimDB::print(){
    nvs_stats_t nvs_stats;
    esp_err_t err = nvs_get_stats(NULL, &nvs_stats);
    if (err != ESP_OK) {
        Debug::error("Błąd odczytu statystyk NVS!");
    } else {
        Debug::info("[NVS Stats]");
        Debug::raw("\tFree: " +  String(nvs_stats.free_entries));
        Debug::raw("\tUsed: " +  String(nvs_stats.used_entries));
        Debug::raw("\tTotal: " +  String(nvs_stats.total_entries) + "\n");
    }
    FileManager::getFreeSpace();
}

void AnimDB::clear(){
    resetNames();
    preferences.clear();
    FileManager::removeAllFiles();
}

 boolean AnimDB::addAnimation(const char* name, const byte* data, size_t length){
    resetNames();
    unsigned short animNum = prepareAnimArr(true);
    if(animNum == 0){
        unsigned short id = 0;
        animArr[0].id = id;
        animArr[0].length = length;
        if(!addAnimationName(id,name)){
            return false;
        }
        if(!addAnimationData(id, data, length)){
            return false;
        }
        saveAnimArr(++animNum);
    }
    else if(preferences.isKey(ANIM_ARR_KEY)){
        unsigned short id = getAnimationId(name, animNum);
        if(id == USHRT_MAX){ 
            id = getNewId(animNum);
            if(id == USHRT_MAX){
                Debug::error("Failed creating new id\n");
                return false;
            }
            if(!addAnimationName(id,name)){
                return false;
            }
            animArr[animNum].id = id;
            animArr[animNum].length =  length;
            animNum++;
        }
        if(!addAnimationData(id, data, length)){
            return false;
        }
        saveAnimArr(animNum);
    }
    Debug::info("Successfully added animation\n");
    return true;
 }

 boolean AnimDB::addAnimationName(unsigned short id, const char* name){
    if(!createKey(ANIM_NAME_KEY, id)){
        Debug::error("Failed creating key\n");
        return false;
    }
    size_t size = preferences.putString(tempKey, name);
    if(size == 0){
        Debug::error("Failed to save name (key : " + String(tempKey) + ")\n");
        return false;
    }
    Debug::info("Successfully saved animation name\n");
    return true;
 }

 boolean AnimDB::addAnimationData(unsigned short id, const byte* data, size_t length){
    if(!createKey(ANIM_PATH_PREFIX, id, ANIM_PATH_SUFFIX)){
        Debug::error("Failed creating key\n");
        return false;
    }
    if(!FileManager::writeFile(tempKey, data, length)){
        Debug::error("Failed to save animation data (key : " + String(tempKey) + ")\n");
        return false;
    }
    Debug::info("Successfully saved animation data\n");
    return true;
 }

boolean AnimDB::addIdToKey(unsigned short id){
    if(tempKey == NULL){
        Debug::error("Cannot add ID to not existing key\n");
        return false;
    }
    int keyEnd = -1;
    for(int i=0; i<MAX_KEY_SIZE; i++){
        if(tempKey[i] == '\0'){
            keyEnd = i;
            break;
        }
    }
    if(keyEnd == -1){
        Debug::error("End of key not found\n");
        return false;
    }
    keyEnd = sprintf(tempKey + keyEnd, "%u", id); //using keyEnd as an error check
    if(keyEnd <= 0){
        Debug::error("Failed to add ID to key\n");
        return false;
    }
    return true;
}

boolean AnimDB::createKey(const char* pref){
    strcpy(tempKey, pref);
    return true;
}
boolean AnimDB::createKey(const char* pref, unsigned short id){
    createKey(pref);
    return addIdToKey(id);
}
boolean AnimDB::createKey(const char* pref, unsigned short id, const char* suff){
    if(createKey(pref, id)){
        strcat(tempKey, ANIM_PATH_SUFFIX);
        return true;
    }
    return false;
}
unsigned short AnimDB::getAnimationId(const char* name, unsigned short animNum){
    unsigned short id;
    for(unsigned short i=0; i < animNum; i++){
        id = animArr[i].id;
        if(!createKey(ANIM_NAME_KEY, id)){
            Debug::error("Failed creating key\n");
            return USHRT_MAX;
        }
        tempName = preferences.getString(tempKey);
        Debug::info("comparing ["+ String(tempName) + " , "+ String(name) + "]\n");
        if(strcmp(name, tempName.c_str()) == 0){
            Debug::info("Found existing name, ID: " + String(id) + "\n");
            return id;
        }
    }
    return USHRT_MAX;
}
unsigned short AnimDB::getNewId(unsigned short animNum){
    unsigned short id;
    boolean found;
    for(id = 0; id < ID_MAX_VALUE; id++){
        found = false;
        for(unsigned short i = 0; i < animNum; i++){
            if(animArr[i].id == id){
                found = true;
                break;
            }
        }
        if(!found){
            Debug::info("Found available id\n");
            return id;
        }
    }
    return USHRT_MAX;
}
boolean AnimDB::getAnimationData(unsigned short id, byte* buff, size_t length){
    if(!createKey(ANIM_PATH_PREFIX, id, ANIM_PATH_SUFFIX)){
        Debug::error("Failed creating key\n");
        return false;
    }
    if(!FileManager::readFile(tempKey, buff, length)){
        Debug::error("Failed to read animation data (key : " + String(tempKey) + ")\n");
        return false;
    }
    Debug::info("Successfully read animation data\n");
    return true;
}
size_t AnimDB::getAnimationSize(const char* name){
    unsigned short animNum = prepareAnimArr(true);
    if(animNum == 0){
        Debug::info("AnimDB is empty\n");
        return 0;
    }
    unsigned short id = getAnimationId(name, animNum);
    if(id == USHRT_MAX){ 
        Debug::error("Coudnt find animation name\n");
        return 0;
    }
    for(unsigned short i = 0; i < animNum; i++){
        if(animArr[i].id == id){
            return animArr[i].length;
        }
    }
    return 0;
}

boolean AnimDB::getAnimation(const char* name, byte* buff, size_t length){
    unsigned short animNum = prepareAnimArr(true);
    if(animNum == 0){
        Debug::info("AnimDB is empty\n");
        return 0;
    }
    unsigned short id = getAnimationId(name, animNum);
    if(id == USHRT_MAX){ 
        Debug::error("Coudnt find animation name\n");
        return false;
    }
    if(!getAnimationData(id, buff, length)){
        return false;
    }
    return true;
}

boolean AnimDB::removeAnimation(const char* name){
    resetNames();
    unsigned short animNum = prepareAnimArr(true);
    if(animNum == 0){
        Debug::info("AnimDB is empty\n");
        return 0;
    }
    unsigned short id = getAnimationId(name, animNum);
    if(id == USHRT_MAX){ 
        Debug::error("Coudnt find animation name\n");
        return false;
    }
    if(!removeAnimationData(id)){
        return false;
    }
    if(!removeAnimationName(id)){
        return false;
    }
    bool found = false;
    for(unsigned short i=0; i<animNum; i++){
        if(!found){
            if(animArr[i].id == id)found = true;
        }
        else{
            animArr[i-1] = animArr[i];
        }
    }
    saveAnimArr(--animNum);
    return true;
}

boolean AnimDB::removeAnimationData(unsigned short id){
    if(!createKey(ANIM_PATH_PREFIX, id, ANIM_PATH_SUFFIX)){
        Debug::error("Failed creating key\n");
        return false;
    }
    if(!FileManager::removeFile(tempKey)){
        Debug::error("Failed to remove animation data (key : " + String(tempKey) + ")\n");
        return false;
    }
    Debug::info("Successfully remove animation data\n");
    return true;
}

boolean AnimDB::removeAnimationName(unsigned short id){
    if(!createKey(ANIM_NAME_KEY, id)){
        Debug::error("Failed creating key\n");
        return false;
    }
    if(!preferences.isKey(tempKey)){
        Debug::error("Failed to remove animation name (key : " + String(tempKey) + ")\n");
        return false;
    }
    preferences.remove(tempKey);
    Debug::info("Successfully remove animation name\n");
    return true;
}

boolean AnimDB::getAllAnimationNames(char* buffer, size_t bufferLength) {

    if(namesLen){
        memcpy(buffer, names, namesLen);
        return true;
    }

    resetNames();

    unsigned short animNum = prepareAnimArr(true);
    if(animNum == 0){
        buffer[0] = '\0';
        return;
    }
    
    size_t totalLength = 0;
    
    for (unsigned short i = 0; i < animNum; i++) {
        if (!createKey(ANIM_NAME_KEY, animArr[i].id)) {
            Debug::error("Failed creating key\n");
            return false;
        }
        tempName = preferences.getString(tempKey);

        size_t nameLength = tempName.length();
        if (totalLength + nameLength + 1 >= bufferLength) {
            Debug::error("Buffer size too small to hold all animation names\n");
            return false;
        }

        strcpy(buffer + totalLength, tempName.c_str());
        totalLength += nameLength;

        if (i < animNum - 1) {
            buffer[totalLength++] = ',';
        }
    }

    buffer[totalLength++] = '\0';

    setNames(buffer, totalLength);

    return true;
}

SizeInfo AnimDB::getSizeInfo(){
    SizeInfo sizeInfo;
    sizeInfo.totalBytes = SPIFFS.totalBytes();
    sizeInfo.usedBytes = SPIFFS.usedBytes();
    return sizeInfo;
}

void AnimDB::setNames(char* buffer, size_t len){
    names = (char*)malloc(len);
    memcpy(names, buffer, len);
    namesLen = len;
    Debug::info("NAMES LEN: " + String(len));
}

void AnimDB::resetNames(){
    if(names != NULL) free(names);
    names = NULL;
    namesLen = 0;
}