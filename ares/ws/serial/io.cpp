auto Serial::readIO(n16 address) -> n8 {
  n8 data;

  switch(address) {

  case 0x00b1:  //SER_DATA
    data = state.dataRx;
    state.rxFull = 0;
    break;

  case 0x00b3:  //SER_STATUS
    data.bit(0) = state.rxFull;
    data.bit(1) = state.rxOverrun;
    data.bit(2) = !state.txFull;
    data.bit(6) = state.baudRate;
    data.bit(7) = state.enable;
    break;

  }

  return data;
}

auto Serial::writeIO(n16 address, n8 data) -> void {
  switch(address) {

  case 0x00b1:  //SER_DATA
    if(!state.txFull) {
      state.dataTx = data;
      state.txFull = 1;
    }
    break;

  case 0x00b3:  //SER_STATUS
    state.rxOverrun &= ~data.bit(5);
    state.baudRate  = data.bit(6);
    state.enable    = data.bit(7);
    break;

  }

  return;
}
