#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdint>
#include <unordered_map>
#include <boost/optional.hpp>
#include <sstream>
#include <cstring>



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
        template<class V>
        void Reflect(V v)const{
                v("sh_name"     , sh_name);
                v("sh_type"     , sh_type);
                v("sh_flags"    , sh_flags);
                v("sh_addr"     , sh_addr);
                v("sh_offset"   , sh_offset);
                v("sh_size"     , sh_size);
                v("sh_link"     , sh_link);
                v("sh_info"     , sh_info);
                v("sh_addralign", sh_addralign);
                v("sh_entsize"  , sh_entsize);  
        }
} Elf64_Shdr;

typedef struct
{
        Elf64_Word  p_type;   /* Type of segment */
        Elf64_Word  p_flags;  /* Segment attributes */
        Elf64_Off   p_offset; /* Offset in file */
        Elf64_Addr  p_vaddr;  /* Virtual address in memory */
        Elf64_Addr  p_paddr;  /* Reserved */
        Elf64_Xword p_filesz; /* Size of segment in file */
        Elf64_Xword p_memsz;  /* Size of segment in memory */
        Elf64_Xword p_align;  /* Alignment of segment */
} Elf64_Phdr;

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
                       std::string const& desc = std::string{}):
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
                        std::stringstream msg;
                        msg << "no default case for " << val;
                        throw std::domain_error(msg.str());
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
                auto d = default_;
                if( ! d )
                        d = Glyph{static_cast<T>(0), "__unknown__"};
                return EnumSwitch{ glyphs_ , d};
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

enum ElfSHT{
        SHT_NULL=0,
        SHT_PROGBITS,
        SHT_SYMTAB,
        SHT_STRTAB,
        SHT_RELA,
        SHT_HASH,
        SHT_DYNAMIC,
        SHT_NOTE,
        SHT_NOBITS,
        SHT_REL,
        SHT_SHLIB,
        SHT_DYNSYM,
        SHT_LOOS=0x60000000,
        SHT_HIOS=0x6FFFFFFF,
        SHT_LOPROC=0x70000000,
        SHT_HIPROC=0x7FFFFFFF,
};
static auto ElfSectionType = EnumSwitchBuilder<Elf64_Word>{}
        .Case(0, "SHT_NULL", "Marks an unused section header")
        .Case(1, "SHT_PROGBITS", "Contains information defined by the program")
        .Case(2, "SHT_SYMTAB", "Contains a linker symbol table")
        .Case(3, "SHT_STRTAB", "Contains a string table")
        .Case(4, "SHT_RELA", "Contains “Rela” type relocation entries")
        .Case(5, "SHT_HASH", "Contains a symbol hash table")
        .Case(6, "SHT_DYNAMIC", "Contains dynamic linking tables")
        .Case(7, "SHT_NOTE", "Contains note information")
        .Case(8, "SHT_NOBITS", "Contains uninitialized space; does not occupy any space in the file")
        .Case(9, "SHT_REL", "Contains “Rel” type relocation entries")
        .Case(10, "SHT_SHLIB", "Reserved")
        .Case(11, "SHT_DYNSYM", "Contains a dynamic loader symbol table")
        .Case(0x60000000, "SHT_LOOS", "Environment-specific use")
        .Case(0x6FFFFFFF, "SHT_HIOS" )
        .Case(0x70000000, "SHT_LOPROC", "Processor-specific use")
        .Case(0x7FFFFFFF, "SHT_HIPROC")
        .Make();



struct PrettyPrinterContext{
        std::vector<char> symbol_table;
};

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
        void DisplayProgramHeaderTable( Elf64_Shdr const& header){}
        void DisplaySectionHeaderTable(PrettyPrinterContext const& ctx, Elf64_Shdr const& header){
                std::cout << "sh_type"  << ElfSectionType(header.sh_type).Name() << "\n";
                std::cout << "sh_name"  << &ctx.symbol_table.at(header.sh_name) << "\n";
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


                std::vector<Elf64_Shdr> section_headers;

                if( header.e_shoff != 0 ){
                        size_t memory_size = header.e_shentsize * header.e_shnum;
                        std::vector<char> raw;
                        raw.resize( memory_size );
                        is.seekg(header.e_shoff, is.beg );
                        std::cout << "header.e_shentsize = " << header.e_shentsize << "\n";
                        std::cout << "sizeof(Elf64_Shdr) = " << sizeof(Elf64_Shdr) << "\n";
                        is.read( reinterpret_cast<char*>(&raw[0]), raw.size());
                        if( ! is ){
                                return "Unable to section header table";
                        }
                        char* ptr = &raw[0];
                        for(size_t idx=0;idx!=header.e_shnum;++idx, ptr += header.e_shentsize){
                                Elf64_Shdr* sh = reinterpret_cast<Elf64_Shdr*>(ptr);
                                section_headers.push_back(*sh);
                                
                        }
                }

                std::vector<char> symbol_table_raw;
                std::vector<std::string> symbol_table;
                
                for(auto const& _ : section_headers ){
                        switch(_.sh_type){
                        case SHT_STRTAB:
                                //sh_offset sh_size  
                                do{
                                        is.seekg(_.sh_offset, is.beg );
                                        symbol_table_raw.resize(_.sh_size);
                                        is.read( &symbol_table_raw[0], symbol_table_raw.size());

                                        auto iter = &symbol_table_raw[0];
                                        auto end = iter + symbol_table_raw.size();
                                        for(; iter != end; iter += strlen(iter)+1 ){
                                                std::string tmp = iter;
                                                symbol_table.push_back(tmp);
                                                //std::cout << "str = " << iter << "\n";

                                        }

                                }while(0);
                        }
                }

                if( true ){
                        is.seekg(header.e_shstrndx, is.beg );
                        
                }
                PrettyPrinterContext pctx;
                pctx.symbol_table = symbol_table_raw;

                PrettyPrinter pp;
                pp.DisplayHeader(header);
                for(auto const& _ : section_headers ){
                        pp.DisplaySectionHeaderTable(pctx, _);

                        
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
