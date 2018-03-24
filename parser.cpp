#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdint>
#include <unordered_map>
#include <boost/optional.hpp>



namespace celf{

//                                     Size |Allign| 
//                                    ------+------+-------------------------
using Elf64_Addr   = void*;         //  8   |    8 | Unsigned program address
using Elf64_Off    = std::uint64_t; //  8   |    8 | Unsigned file offset
using Elf64_Half   = std::uint16_t; //  2   |    2 | Unsigned medium integer
using Elf64_Word   = std::uint32_t; //  4   |    4 | Unsigned integer
using Elf64_Sword  = std::int32_t;  //  4   |    4 | Signed integer
using Elf64_Xword  = std::uint64_t; //  8   |    8 | Unsigned long integer
using Elf64_Sxword = std::int64_t;  //  8   |    8 | Signed long integer

typedef struct
{
	unsigned char e_ident[16]; /* ELF identification */
	Elf64_Half e_type;         /* Object file type */
	Elf64_Half e_machine;      /* Machine type */
	Elf64_Word e_version;      /* Object file version */
	Elf64_Addr e_entry;        /* Entry point address */
	Elf64_Off e_phoff;         /* Program header offset */
	Elf64_Off e_shoff;         /* Section header offset */
	Elf64_Word e_flags;        /* Processor-specific flags */
	Elf64_Half e_ehsize;       /* ELF header size */
	Elf64_Half e_phentsize;    /* Size of program header entry */
	Elf64_Half e_phnum;        /* Number of program header entries */
	Elf64_Half e_shentsize;    /* Size of section header entry */
	Elf64_Half e_shnum;        /* Number of section header entries */
	Elf64_Half e_shstrndx;     /* Section name string table index */

        template<class V>
        void Reflect(V v)const{
                // v("e_ident", e_ident);
                v("e_type", e_type);
                v("e_machine", e_machine);
                v("e_version", e_version);
                v("e_entry", e_entry);
                v("e_phoff", e_phoff);
                v("e_shoff", e_shoff);
                v("e_flags", e_flags);
                v("e_ehsize", e_ehsize);
                v("e_phentsize", e_phentsize);
                v("e_phnum", e_phnum);
                v("e_shentsize", e_shentsize);
                v("e_shnum", e_shnum);
                v("e_shstrndx", e_shstrndx);  
        }
} Elf64_Ehdr;
enum ElfIE{
//          Name           Value | Purpose
//      ----------------+--------+----------------
        EI_MAG0=0,     //    0   | File identification
        EI_MAG1,       //    1   |
        EI_MAG2,       //    2   |
        EI_MAG3,       //    3   |
        EI_CLASS,      //    4   | File class
        EI_DATA,       //    5   | Data encoding
        EI_VERSION,    //    6   | File version
        EI_OSABI,      //    7   | OS/ABI identification
        EI_ABIVERSION, //    8   | ABI version
        EI_PAD,        //    9   | Start of padding bytes
        EI_NIDENT=16,  //   16   | Size of e_ident[]
};

typedef struct
{
        Elf64_Word  sh_name;      /* Section name */
        Elf64_Word  sh_type;      /* Section type */
        Elf64_Xword sh_flags;     /* Section attributes */
        Elf64_Addr  sh_addr;      /* Virtual address in memory */
        Elf64_Off   sh_offset;    /* Offset in file */
        Elf64_Xword sh_size;      /* Size of section */
        Elf64_Word  sh_link;      /* Link to other section */
        Elf64_Word  sh_info;      /* Miscellaneous information */
        Elf64_Xword sh_addralign; /* Address alignment boundary */
        Elf64_Xword sh_entsize;   /* Size of entries, if section has table */
} Elf64_Shdr

#if 0
enum ElfClass{
//       Name          | Value |     Meaning
//      ---------------+-------+---------------
        ELFCLASS32=1, //   1   | 32-bit objects
        ELFCLASS64,   //   2   | 64-bit objects
};

enum ElfData{
        ELFDATA2LSB=1, // 1 Object file data structures are littleendian
        ELFDATA2MSB,   // 2 Object file data structures are bigendian
};
#endif


template<class T>
struct EnumSwitchBuilder{
        struct Glyph{
                Glyph( T value, std::string const& name,
                       std::string const& desc):
                        value_{value}, name_{name}, 
                        desc_{desc}
                {}
                auto Value()const{ return value_; }
                auto const& Name()const{ return name_; }
                auto const& Description()const{ return desc_; }
                operator std::string const&()const{
                        return value_;
                }
        private:
                T value_;
                std::string name_;
                std::string desc_;         
        };
        EnumSwitchBuilder& Case(T value,
                                std::string const& name,
                                std::string const& desc = std::string{}){
                glyphs_.emplace_back(value, name, desc);
                return *this;
        }
        EnumSwitchBuilder& Default(std::string const& name){
                if( !! default_ )
                        throw std::domain_error("can only have one default");
                default_ = Glyph{0, name, ""};
                return *this;
        }
        struct EnumSwitch{
                explicit EnumSwitch(std::vector< Glyph > const& glyphs,
                                    boost::optional<Glyph> const& _default )
                        : mem_{ glyphs } , default_{_default}
                {
                        for(auto& _ : mem_ ){
                                from_val_.emplace( _.Value(), &_);
                                from_str_.emplace( _.Name(), &_);
                        }
                }
                Glyph const& Switch(T val)const{
                        auto iter = from_val_.find(val);
                        if( iter != from_val_.end() )
                                return *iter->second;
                        if( !! default_ )
                                return default_.get();
                        throw std::domain_error("no default case");
                }
                auto const& operator()(T val)const{
                        return this->Switch(val);
                }
        private:
                std::vector< Glyph > mem_;
                boost::optional<Glyph> default_;
                std::unordered_map<T, Glyph* > from_val_;
                std::unordered_map<std::string, Glyph* > from_str_;
        };
        auto Make(){
                return EnumSwitch{ glyphs_ , default_};
        }

private:
        std::vector<Glyph> glyphs_;
        boost::optional<Glyph> default_;
};

static auto ElfDataClass = EnumSwitchBuilder<unsigned char>{}
        .Case(1, "ELFCLASS32", "32-bit objects")
        .Case(2, "ELFCLASS64", "64-bit objects")
        .Make();

static auto ElfDataEncoding = EnumSwitchBuilder<unsigned char>{}
        .Case(1, "ELFDATA2LSB", "Object file data structures are littleendian")
        .Case(2, "ELFDATA2MSB", "Object file data structures are bigendian")
        .Make();

static auto ElfOSABI = EnumSwitchBuilder<unsigned char>{}
        .Case(  0, "ELFOSABI_SYSV", "System V ABI")
        .Case(  1, "ELFOSABI_HPUX", "HP-UX operating system")
        .Case(255, "ELFOSABI_HPUX", "ELFOSABI_STANDALONE 255 Standalone (embedded)")
        .Make();

static auto ElfObjectType = EnumSwitchBuilder<Elf64_Half>{}
        .Case(0,      "ET_NONE",   "No file type")
        .Case(1,      "ET_REL",    "Relocatable object file")
        .Case(2,      "ET_EXEC",   "Executable file")
        .Case(3,      "ET_DYN",    "Shared object file")
        .Case(4,      "ET_CORE",   "Core file")
        .Case(0xFE00, "ET_LOOS",   "Environment-specific use")
        .Case(0xFEFF, "ET_HIOS")
        .Case(0xFF00, "ET_LOPROC", "Processor-specific use")
        .Case(0xFFFF, "ET_HIPROC")
        .Make();

struct PrettyPrinter{
        void DisplayHeader( Elf64_Ehdr const& header)const{
                std::string magic(4, ' ');
                magic[0] = header.e_ident[EI_MAG0];
                magic[1] = header.e_ident[EI_MAG1];
                magic[2] = header.e_ident[EI_MAG2];
                magic[3] = header.e_ident[EI_MAG3];


                std::cout << "magic = " << magic << "\n";
                std::cout << "e_ident[EI_CLASS] = " << ElfDataClass(header.e_ident[EI_CLASS]).Name() << "\n";
                std::cout << "e_ident[EI_DATA] = " << ElfDataEncoding(header.e_ident[EI_DATA]).Name() << "\n";
                std::cout << "e_ident[EI_OSABI] = " << ElfOSABI(header.e_ident[EI_OSABI]).Name() << "\n";
                std::cout << "e_type = " << ElfObjectType(header.e_type).Name() << "\n";

                header.Reflect( [](auto n, auto v){
                        std::cout << n << ":" << v << "\n";
                });
        }
};

struct ElfParser{
        std::string Parse(std::ifstream& is){
                Elf64_Ehdr header;

                is.seekg(0, is.beg );

                is.read( reinterpret_cast<char*>(&header), sizeof(header));

                if( ! is ){
                        return "Unable to parse header";
                }

                PrettyPrinter pp;
                pp.DisplayHeader(header);


                if( header_.e_phoff != 0 ){
                        is.seekg(0, header_.e_phoff );
                }

                return "";
        }
};

} // end namespace celf

int main(int argc, char** argv){
        using namespace celf;
        if( argc != 2 ){
                std::cerr << "Syntax: " << argv[0] << " <file>\n";
                return EXIT_FAILURE;
        }
        std::string filename = argv[1];
        
        ElfParser parser;
        std::ifstream ifstr( filename, std::ifstream::binary );
        if( ! ifstr.is_open() ){
                std::cerr << "Unable to open " << filename << "\n";
                return EXIT_FAILURE;
        }

        auto pret = parser.Parse( ifstr );
        std::cout << "pret = " << pret << "\n";

        return EXIT_SUCCESS;
}
