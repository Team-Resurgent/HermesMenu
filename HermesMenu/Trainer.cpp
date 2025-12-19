#include "Trainer.h"
#include "External.h"
#include "utils.h"
#include "Defines.h"
#include "StringUtility.h"

uint8_t* Trainer::LoadTrainer(std::string path, uint32_t* trainerSize)
{
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK iostatusBlock;
    HANDLE fileHandle;
    NTSTATUS status;
    FILE_NETWORK_OPEN_INFORMATION fileInfo;
     
    std::string etmPath = StringUtility::FormatString("%s\\trainer.etm", path.c_str());

    STRING etmPathString;
    etmPathString.Buffer = (PSTR)etmPath.c_str();
    etmPathString.Length = strlen(etmPathString.Buffer);
    etmPathString.MaximumLength = etmPathString.Length + 1;

    objectAttributes.Attributes = OBJ_CASE_INSENSITIVE;
    objectAttributes.ObjectName = &etmPathString;
    objectAttributes.RootDirectory = 0;

    status = NtOpenFile(&fileHandle, GENERIC_READ | SYNCHRONIZE, &objectAttributes, &iostatusBlock, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE);
    if (status != 0) 
    {
        return NULL;
    }

    status = NtQueryInformationFile(fileHandle, &iostatusBlock, &fileInfo, sizeof(fileInfo), FileNetworkOpenInformation);
    if (status != 0) 
    {
        NtClose(fileHandle);
        return NULL;
    }

    *trainerSize = fileInfo.EndOfFile.LowPart;
    if (fileInfo.EndOfFile.HighPart > 0) 
    {
        NtClose(fileHandle);
        return NULL;
    }

    uint8_t* trainerData = (uint8_t*)malloc(*trainerSize);
    status = NtReadFile(fileHandle, NULL, NULL, NULL, &iostatusBlock, trainerData, *trainerSize, NULL);
    NtClose(fileHandle);

    if (status != 0)
    {
        free(trainerData);
        return NULL;
    }
    return trainerData;
}

std::vector<std::string> Trainer::GetOptions(uint8_t* trainerData, uint32_t trainerSize)
{
    std::vector<std::string> result;
    
    uint32_t textOffset = *((uint32_t*)(trainerData + ETM_SELECTIONS_TEXT_OFFSET));
    uint32_t optionOffset = *((uint32_t*)(trainerData + ETM_SELECTIONS_OFFSET));

    uint32_t optionCount = textOffset - optionOffset;

    char temp[85];
    for (uint32_t i = 0; i < optionCount + 2; i++)
    {
        uint32_t offset;
        memcpy(&offset, trainerData + textOffset + 4 * i, 4);
        if (!offset)
        {
            break;
        }

        if (offset > trainerSize || textOffset + 4 * i > trainerSize)
        {
            return result;
        }
        strcpy(temp, (char*)(trainerData + offset));
        result.push_back(std::string(temp));
    }

    return result;
}

void Trainer::ListTitleids(uint8_t* trainerData, uint32_t trainerSize)
{
    uint32_t offset;
    uint8_t* list = trainerData + ETM_ID_LIST; 
    memcpy(&offset, list, 4);

    for (uint32_t i = 0; i < 3; i++)
    {
        uint32_t titleid;
        memcpy(&titleid, trainerData + offset, 4);
        utils::debugPrint("Title ID %i: %08X\n", i, titleid);
        offset += 4;
    }
}

bool Trainer::GetOption(uint8_t* trainerData, uint32_t trainerSize, uint32_t index)
{
    uint32_t textOffset = *((uint32_t*)(trainerData + ETM_SELECTIONS_TEXT_OFFSET));
    uint32_t optionOffset = *((uint32_t*)(trainerData + ETM_SELECTIONS_OFFSET));
    uint32_t optionCount = textOffset - optionOffset;
    if (index >= optionCount)
    {
        return false;
    }
    return trainerData[optionOffset + index] == 1;
}

void Trainer::ToggleOption(uint8_t* trainerData, uint32_t trainerSize, uint32_t index)
{
    uint32_t textOffset = *((uint32_t*)(trainerData + ETM_SELECTIONS_TEXT_OFFSET));
    uint32_t optionOffset = *((uint32_t*)(trainerData + ETM_SELECTIONS_OFFSET));
    uint32_t optionCount = textOffset - optionOffset;
    if (index >= optionCount)
    {
        return;
    }
    trainerData[optionOffset + index] = trainerData[optionOffset + index] == 1 ? 0 : 1;
}