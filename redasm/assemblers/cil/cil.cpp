#include "cil.h"

namespace REDasm {

CILAssembler::CILAssembler() { }
std::string CILAssembler::name() const { return "CIL/MSIL"; }
bool CILAssembler::decodeInstruction(const BufferView& view, const InstructionPtr &instruction) { return false; }

} // namespace REDasm
