#include <ws/ws.hpp>

namespace ares::WonderSwan {

Serial serial;
#include "debugger.cpp"
#include "io.cpp"
#include "serialization.cpp"

auto Serial::load(Node::Object parent) -> void {
  node = parent->append<Node::Object>("Serial");
  debugger.load(node);
}

auto Serial::unload() -> void {
  debugger.unload(node);
  node.reset();
}

auto Serial::read(void) -> i32 {
  return -1;
}

auto Serial::write(n8 byte) -> void {
  if(unlikely(debugger.tracer.comms->enabled())) {
    debugger.tracer.comms->notify({"Write ", hex(byte, 2L)});
  }
}

auto Serial::main() -> void {
  step(80);
  
  if (!state.enable) return;
  if (!state.baudRate && ++state.baudClock < 4) return;
  state.baudClock = 0;

  // transfer
  if(state.txFull) {
    if(++state.txBitClock == 10) {
      write(state.dataTx);
      state.txBitClock = 0;
      state.txFull = 0;
    }
  }

  // receive
  if(++state.rxBitClock == 10) {
    state.rxBitClock = 0;
    auto value = read();
    if(value >= 0) {
      state.dataRx = value;
      if(state.rxFull) {
        // TODO: Is state.dataRx overwritten on overrun?
        if(unlikely(debugger.tracer.comms->enabled())) {
          debugger.tracer.comms->notify({"Read ", hex(value & 0xFF, 2L), " [overrun]"});
        }
        state.rxOverrun = 1;
      } else {
        if(unlikely(debugger.tracer.comms->enabled())) {
          debugger.tracer.comms->notify({"Read ", hex(value & 0xFF, 2L)});
        }
        state.rxFull = 1;
      }
    }
  }
  
  cpu.irqLevel(CPU::Interrupt::SerialSend, !state.txFull);
  cpu.irqLevel(CPU::Interrupt::SerialReceive, state.rxFull);
}

auto Serial::step(u32 clocks) -> void {
  Thread::step(clocks);
  Thread::synchronize(cpu);
}

auto Serial::power() -> void {
  Thread::create(3'072'000, {&Serial::main, this});

  bus.map(this, 0x00b1);
  bus.map(this, 0x00b3);

  state = {};
}

}
