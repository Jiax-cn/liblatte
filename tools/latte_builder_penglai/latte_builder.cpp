#include "latte.h"
#include "latte_wasm.h"
#include "crypto/sha256.h"
#include "utils.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <vector>
#include <tuple>

const char *kOutWasmPrefix = "latte_";
const char *kOutIdSuffix = ".id";
const uint32_t kMaxFileNameLength = 256;

const uint32_t kDefaultPlatformNum = 2;

class LatteGroup
{
public:
    enum PayloadPart {
        PortableIdentity,
        PortablePayload,
    };

    enum CommonPart {
        Latte,
        Mage,
    };

private:

    class WasmIdentity
    {
    public:
        WasmIdentity() {};
        ~WasmIdentity() {};

        const char *name_;
        portid_t portid_;
        portid_hash_state_t portid_state_;
    };

    /*
    * common part(Latte):
    * | num_platform(u32) |
    * | sgx_im_hash | penglai_im_hash |
    * ...
    * common part(Mage):
    * | num_platform(u32) | num_wasm(u32) |
    * | name | sgx_im_hash | penglai_im_hash |
    * | name | sgx_im_hash | penglai_im_hash |
    * ...
    */
    uint8_t rt_common_part_[WASM_COMMON_SEC_SIZE] = {0};
    
    sgx_hash_state_t sgx_meta_hash_state_;
    penglai_hash_state_t penglai_meta_hash_state_;

    uint8_t *wasm_portid_section_ = nullptr;
    uint32_t wasm_portid_section_size_ = 0;

    PayloadPart payload_ = PortableIdentity;
    CommonPart rt_common_ = Latte;

    std::vector<WasmIdentity> wasm_members_;

    void WriteSections(const char *file_name, uint8_t *buf, uint32_t buf_size);

    void UpdateMageCommonPart(WasmIdentity wasm_id, uint8_t *pld_part, uint32_t *offset);

    void GenPortidSection();

public:
    LatteGroup() {};
    ~LatteGroup() {};

    void SetConfig(PayloadPart payload, CommonPart common);

    void LoadSgxIntermediateHash(const char *file_name);

    void LoadPenglaiIntermediateHash(const char *file_name);

    void AddLatteMember(const char *file_name);

    void InsertPortidSection();

    void OutputPortableIdentity();

    void OutputRuntimeCommonPart();

    void PrintMageMeasurement();

    void PrintLatteMeasurement();
};

void LatteGroup::SetConfig(PayloadPart payload, CommonPart common)
{
    payload_ = payload;
    rt_common_ = common;
    return;
}

void LatteGroup::LoadSgxIntermediateHash(const char *file_name)
{
    if (read_file_to_buf(file_name, (uint8_t *)&sgx_meta_hash_state_, sizeof(sgx_hash_state_t)) < sizeof(sgx_hash_state_t))
    {
        printf("Failed to read %s: read file content failed.\n", file_name);
    }
    return;
}

void LatteGroup::LoadPenglaiIntermediateHash(const char *file_name)
{
    if (read_file_to_buf(file_name, (uint8_t *)&penglai_meta_hash_state_, sizeof(penglai_hash_state_t)) < sizeof(penglai_hash_state_t))
    {
        printf("Failed to read %s: read file content failed.\n", file_name);
    }
    return;
}

void LatteGroup::WriteSections(const char *file_name, uint8_t *buf, uint32_t buf_size)
{
    int file;
    uint32_t write_size;

    if ((file = open(file_name, O_RDWR | O_CREAT, S_IRWXO | S_IRWXG | S_IRWXU)) == -1) 
    {
        printf("file open failed: open file %s failed.\n", file_name);
        return;
    }

    write_size = write(file, buf, buf_size);

    if (write_size != buf_size)
    {
        printf("Writing file %s error: size: %d invalid!\n", file_name, write_size);
        return;
    }
    
    close(file);
    return;
}

void LatteGroup::UpdateMageCommonPart(WasmIdentity wasm_id, uint8_t *pld_part, uint32_t *offset)
{
    // new wasm file in common part: | name | 0x00 | im hash ... | ;
    uint32_t name_length = strlen(wasm_id.name_)+1;
    memcpy(rt_common_part_+*offset, wasm_id.name_, name_length);
    *offset += name_length;

    // update extension section: sgx
    sgx_update_common_part(sgx_meta_hash_state_, pld_part, WASM_PLD_SEC_SIZE, 
        reinterpret_cast<sgx_hash_state_t *>(rt_common_part_+*offset));
    *offset += sizeof(sgx_hash_state_t);

    // update extension section: penglai
    penglai_update_common_part(penglai_meta_hash_state_, pld_part, WASM_PLD_SEC_SIZE, 
        reinterpret_cast<penglai_hash_state_t *>(rt_common_part_+*offset));
    *offset += sizeof(penglai_hash_state_t);
    return;
}

void LatteGroup::OutputRuntimeCommonPart()
{
    uint32_t offset = 0;
    std::vector<WasmIdentity>::iterator it;

    switch (rt_common_)
    {
    case Latte:
        *reinterpret_cast<uint32_t *>(rt_common_part_+offset) = kDefaultPlatformNum;
        offset += sizeof(uint32_t);
        memcpy(rt_common_part_+offset, &sgx_meta_hash_state_, sizeof(sgx_hash_state_t));
        offset += sizeof(sgx_hash_state_t);
        memcpy(rt_common_part_+offset, &penglai_meta_hash_state_, sizeof(penglai_hash_state_t));
        offset += sizeof(penglai_hash_state_t);
        break;
    case Mage:
        *reinterpret_cast<uint32_t *>(rt_common_part_+offset) = kDefaultPlatformNum;
        offset += sizeof(uint32_t);
        *reinterpret_cast<uint32_t *>(rt_common_part_+offset) = wasm_members_.size();
        offset += sizeof(uint32_t);
        switch (payload_)
        {
        case PortableIdentity:
            for(it = wasm_members_.begin(); it != wasm_members_.end(); it++)
            {
                uint8_t pld_part[WASM_PLD_SEC_SIZE] = {0};
                memcpy(pld_part, it->portid_, SHA256_DIGEST_LENGTH);
                
                UpdateMageCommonPart(*it, pld_part, &offset);
            }
            break;
        case PortablePayload:
            for(it = wasm_members_.begin(); it != wasm_members_.end(); it++)
            {
                char pld_file_name[kMaxFileNameLength] = {0};
                uint8_t pld_part[WASM_PLD_SEC_SIZE] = {0};
                uint32_t pld_file_size = 0; 

                memcpy(pld_file_name, kOutWasmPrefix, strlen(kOutWasmPrefix));
                memcpy(pld_file_name+strlen(kOutWasmPrefix), it->name_, strlen(it->name_)+1);
                pld_file_size = get_file_size(pld_file_name);

                if (pld_file_size > WASM_PLD_SEC_SIZE-sizeof(uint32_t))
                {
                    printf("Warning: invalid portable payload size.\n");
                }

                *reinterpret_cast<uint32_t*>(pld_part+WASM_PLD_SEC_SIZE-sizeof(uint32_t)) = pld_file_size;
                if (read_file_to_buf(pld_file_name, pld_part+WASM_PLD_SEC_SIZE-sizeof(uint32_t)-pld_file_size, pld_file_size) < pld_file_size) 
                {
                    printf("file open failed: open file %s failed.\n", pld_file_name);
                    return;
                }

                UpdateMageCommonPart(*it, pld_part, &offset);
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return WriteSections("runtime_common.bin", rt_common_part_, offset);
}

void LatteGroup::OutputPortableIdentity()
{
    if (!wasm_portid_section_)
    {
        printf("Warning: portid section is empty.\n");
    }

    std::vector<WasmIdentity>::iterator it;
    for(it = wasm_members_.begin(); it != wasm_members_.end(); it++)
    {
        derive_portid(it->portid_state_, wasm_portid_section_, wasm_portid_section_size_, it->portid_);
        
        char id_file_name[kMaxFileNameLength] = {0};
        memcpy(id_file_name, it->name_, strlen(it->name_));
        memcpy(id_file_name+strlen(it->name_), kOutIdSuffix, strlen(kOutIdSuffix)+1);

        WriteSections(id_file_name, it->portid_, sizeof(portid_t));

        printf("portid of %s:\n", it->name_);
        hexdump(it->portid_, sizeof(portid_t));
    }
    return;
}

void LatteGroup::GenPortidSection()
{
    uint32_t data_size = wasm_members_.size() * sizeof(portid_hash_state_t);
    uint8_t *data = (uint8_t *)malloc(data_size);
    uint32_t offset = 0;

    std::vector<WasmIdentity>::iterator it;
    for(it = wasm_members_.begin(); it != wasm_members_.end(); it++)
    {
        memcpy(data+offset, &it->portid_state_, sizeof(portid_hash_state_t));
        offset += sizeof(portid_hash_state_t);
    }

    wasm_portid_section_ = encode_portid_section(data, data_size, &wasm_portid_section_size_);
    free(data);

    printf("output portid section: \n");
    hexdump(wasm_portid_section_, wasm_portid_section_size_);
    return;
}

void LatteGroup::InsertPortidSection()
{
    GenPortidSection();

    std::vector<WasmIdentity>::iterator it;
    for(it = wasm_members_.begin(); it != wasm_members_.end(); it++)
    {
        const char *raw_file_name = it->name_;
        uint32_t raw_wasm_size = 0;
        uint8_t *raw_wasm = nullptr;

        int out_file = 0;
        uint32_t write_size = 0;
        char out_file_name[kMaxFileNameLength] = {0};

        if ((raw_wasm = read_file(raw_file_name, &raw_wasm_size)) == nullptr) 
        {
            printf("file open failed: open file %s failed.\n", raw_file_name);
            return;
        }

        memcpy(out_file_name, kOutWasmPrefix, strlen(kOutWasmPrefix));
        memcpy(out_file_name+strlen(kOutWasmPrefix), raw_file_name, strlen(raw_file_name)+1);

        if ((out_file = open(out_file_name, O_RDWR | O_CREAT, S_IRWXO | S_IRWXG | S_IRWXU)) == -1) 
        {
            printf("file open failed: open file %s failed.\n", out_file_name);
            return;
        }

        write_size = write(out_file, raw_wasm, raw_wasm_size);
        // printf("write %u to %s\n", raw_wasm_size, out_file_name);

        if (write_size != raw_wasm_size)
        {
            printf("Writing WASM source error: size: %d invalid!\n", write_size);
            return;
        }

        write_size = write(out_file, wasm_portid_section_, wasm_portid_section_size_);
        // printf("write %u to %s\n", wasm_portid_section_size_, out_file_name);

        if (write_size != wasm_portid_section_size_)
        {
            printf("Writing portmr section error: size: %d invalid!\n", write_size);
            return;
        }

        close(out_file);
    }
    return;
}

void LatteGroup::AddLatteMember(const char *file_name)
{
    uint8_t *raw_wasm = nullptr; 
    uint32_t raw_wasm_size = 0;
    WasmIdentity wasm_id = WasmIdentity();

    raw_wasm = read_file(file_name, &raw_wasm_size);

    wasm_id.name_ = file_name;
    gen_portid_state(raw_wasm, raw_wasm_size, wasm_id.portid_state_);
    wasm_members_.push_back(wasm_id);
    free(raw_wasm);
    return;
}

void LatteGroup::PrintMageMeasurement()
{
    uint32_t offset = 2 * sizeof(uint32_t);
    char *name = nullptr;
    latte_sgx_measurement_t sgx_mr;
    latte_penglai_measurement_t penglai_mr;

    for (size_t i = 0; i < wasm_members_.size(); i++)
    {
        name = reinterpret_cast<char *>(rt_common_part_ + offset);
        printf("application %s's measurement:\n", name);
        offset += strlen(name) + 1;

        sgx_derive_common_part(*reinterpret_cast<sgx_hash_state_t *>(rt_common_part_ + offset),
            rt_common_part_, sgx_mr);
        offset += sizeof(sgx_hash_state_t);

        printf("sgx:  ");
        hexdump(sgx_mr, SHA256_DIGEST_LENGTH);
        
        penglai_derive_common_part(*reinterpret_cast<penglai_hash_state_t *>(rt_common_part_ + offset),
            rt_common_part_, 0, penglai_mr);
        offset += sizeof(penglai_hash_state_t);

        printf("penglai:  ");
        hexdump(penglai_mr, SM3_DIGEST_LENGTH);
    }
    return;
}

void LatteGroup::PrintLatteMeasurement()
{
    uint32_t offset = sizeof(uint32_t);
    sgx_hash_state_t *sgx_state = nullptr;
    penglai_hash_state_t *penglai_state = nullptr;
    latte_sgx_measurement_t sgx_mr;
    latte_penglai_measurement_t penglai_mr;

    sgx_state = reinterpret_cast<sgx_hash_state_t *>(rt_common_part_ + offset);
    offset += sizeof(sgx_hash_state_t);

    penglai_state = reinterpret_cast<penglai_hash_state_t *>(rt_common_part_ + offset);
    offset += sizeof(penglai_hash_state_t);

    if (payload_ == PortablePayload)
    {
        sgx_init_state_list(*sgx_state);
        penglai_init_state_list(*penglai_state);
    }

    std::vector<WasmIdentity>::iterator it;
    for(it = wasm_members_.begin(); it != wasm_members_.end(); it++)
    {
        const char *name = it->name_;

        uint32_t pld_file_size = 0;
        uint8_t *pld_file = nullptr;
        char pld_file_name[kMaxFileNameLength] = {0};

        printf("application %s's measurement:\n", name);
        switch (payload_)
        {
        case PortablePayload:
            memcpy(pld_file_name, kOutWasmPrefix, strlen(kOutWasmPrefix));
            memcpy(pld_file_name+strlen(kOutWasmPrefix), name, strlen(name)+1);

            if ((pld_file = read_file(pld_file_name, &pld_file_size)) == nullptr) 
            {
                printf("file open failed: open file %s failed.\n", pld_file_name);
                return;
            }
            sgx_derive_hardcode_wasm(pld_file, pld_file_size, rt_common_part_, sgx_mr);
            penglai_derive_hardcode_wasm(pld_file, pld_file_size, rt_common_part_, 0, penglai_mr);

            break;
        case PortableIdentity:
            sgx_derive_hardcode_portid(*sgx_state, it->portid_, rt_common_part_, sgx_mr);
            penglai_derive_hardcode_portid(*penglai_state, it->portid_, rt_common_part_, 0, penglai_mr);
            break;
        default:
            break;
        }

        printf("sgx:  ");
        hexdump(sgx_mr, SHA256_DIGEST_LENGTH);
        
        printf("penglai:  ");
        hexdump(penglai_mr, SM3_DIGEST_LENGTH);
    }

    return;
}

int main(int argc, char *argv[])
{

    const char *im_hash_files[2] = {"sgx_rt_mr.bin", "penglai_vm_mr.bin"};

    LatteGroup *latte_group = new LatteGroup();

    latte_group->LoadSgxIntermediateHash(im_hash_files[0]);
    latte_group->LoadPenglaiIntermediateHash(im_hash_files[1]);

    for (int i = 1; i < argc; i++)
    {
        latte_group->AddLatteMember(argv[i]);
    }

    latte_group->InsertPortidSection();

{
    latte_group->SetConfig(LatteGroup::PortableIdentity, LatteGroup::Latte);
    latte_group->OutputPortableIdentity();
    latte_group->OutputRuntimeCommonPart();

    latte_group->PrintLatteMeasurement();
}

// {
//     latte_group->SetConfig(LatteGroup::PortablePayload, LatteGroup::Latte);
//     latte_group->OutputRuntimeCommonPart();

//     latte_group->PrintLatteMeasurement();
// }

// {
//     latte_group->SetConfig(LatteGroup::PortableIdentity, LatteGroup::Mage);
//     latte_group->OutputPortableIdentity();
//     latte_group->OutputRuntimeCommonPart();

//     latte_group->PrintMageMeasurement();
// }

// {
//     latte_group->SetConfig(LatteGroup::PortablePayload, LatteGroup::Mage);
//     latte_group->OutputRuntimeCommonPart();

//     latte_group->PrintMageMeasurement();
// }
    
    return 0;
}
