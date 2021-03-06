#include "psxexe.h"
#include "psxexe_analyzer.h"
#include <cstring>

#define PSXEXE_SIGNATURE   "PS-X EXE"
#define PSXEXE_TEXT_OFFSET 0x00000800
#define PSX_USER_RAM_START 0x80000000
#define PSX_USER_RAM_END   0x80200000

namespace REDasm {

FORMAT_PLUGIN_TEST(PsxExeFormat, PsxExeHeader) { return !strncmp(format->id, PSXEXE_SIGNATURE, PSXEXE_SIGNATURE_SIZE); }

PsxExeFormat::PsxExeFormat(AbstractBuffer *buffer): FormatPluginT<PsxExeHeader>(buffer) { }
std::string PsxExeFormat::name() const { return "PS-X Executable"; }
u32 PsxExeFormat::bits() const { return 32; }
std::string PsxExeFormat::assembler() const { return "mips32le"; }
Analyzer *PsxExeFormat::createAnalyzer(DisassemblerAPI *disassembler, const SignatureFiles &signatures) const { return new PsxExeAnalyzer(disassembler, signatures); }

void PsxExeFormat::load()
{
    m_signatures.push_back("psyq47");

    if(m_format->t_addr > PSX_USER_RAM_START)
        m_document->segment("RAM0", 0, PSX_USER_RAM_START, (m_format->t_addr - PSX_USER_RAM_START), SegmentTypes::Bss);

    m_document->segment("TEXT", PSXEXE_TEXT_OFFSET, m_format->t_addr, m_format->t_size, SegmentTypes::Code | SegmentTypes::Data);

    if((m_format->t_addr + m_format->t_size) < PSX_USER_RAM_END)
        m_document->segment("RAM1", 0, m_format->t_addr + m_format->t_size, PSX_USER_RAM_END - (m_format->t_addr + m_format->t_size), SegmentTypes::Bss);

    m_document->entry(m_format->pc0);
}

}
