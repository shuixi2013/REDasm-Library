#include "emulator.h"
#include "../disassembler/listing/listingdocument.h"
#include "format.h"
#include <cstring>

namespace REDasm {

Emulator::Emulator(DisassemblerAPI *disassembler): m_disassembler(disassembler) { this->remap(); }

void Emulator::emulate(const InstructionPtr &instruction)
{
    m_currentinstruction = instruction;

    if(instruction->is(InstructionTypes::Branch) && instruction->hasTargets())
        this->setTarget(instruction);

    m_dispatcher(instruction->id, instruction);
}

bool Emulator::setTarget(const InstructionPtr &instruction)
{
   if(instruction->target_idx == -1)
       return false;

   const Operand* op = instruction->targetOperand();

   if(!op || !op->is(OperandTypes::Register))
       return false;

   u64 value = 0;

   if(!this->read(op, &value))
       return false;

   instruction->target(value);
   m_disassembler->document()->update(instruction);
   return true;
}

MemoryBuffer *Emulator::getSegmentMemory(address_t address, offset_t *offset)
{
    for(auto it = m_memory.begin(); it != m_memory.end(); it++)
    {
        const Segment* segment = it->first;

        if(!segment->contains(address))
            continue;

        *offset = (address - segment->address); // Relative segment offset
        return it->second.get();
    }

    return nullptr;
}

BufferView Emulator::getMemory(address_t address)
{
    offset_t offset = 0;
    MemoryBuffer* buffer = this->getSegmentMemory(address, &offset);

    if(!buffer || buffer->empty())
        return BufferView();

    return buffer->view(offset);
}

BufferView Emulator::getStack(offset_t sp) { return m_stack->view(sp); }

void Emulator::remap()
{
    auto& document = m_disassembler->document();
    FormatPlugin* format = m_disassembler->format();

    REDasm::log("MAPPING 'stack'");
    m_stack = std::make_unique<MemoryBuffer>(STACK_SIZE, 0);
    m_memory.clear();

    for(size_t i = 0; i < document->segmentsCount(); i++)
    {
        const Segment* segment = document->segmentAt(i);

        REDasm::log("MAPPING " + REDasm::quoted(segment->name) +
                    " @ " + REDasm::hex(segment->address) + ", " +
                    " size: " + REDasm::hex(segment->size()));

        if(!segment->is(SegmentTypes::Bss))
        {
            BufferView view = format->view(segment->address);

            if(segment->size() > static_cast<s64>(view.size()))
                return;

            auto buffer = std::make_unique<MemoryBuffer>();
            view.copyTo(buffer.get());
            m_memory[segment] = std::move(buffer);
        }
        else
            m_memory[segment] = std::make_unique<MemoryBuffer>(segment->size(), 0);
    }
}

} // namespace REDasm
