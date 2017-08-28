#ifndef PRINTER_STATE_SNAPSHOT
#define PRINTER_STATE_SNAPSHOT

#include "libs/StreamOutput.h"

class PrinterStateSnapshot {
public:
    PrinterStateSnapshot(PrinterStateSnapshot& state);
    PrinterStateSnapshot(const PrinterStateSnapshot& state);

    void restore(StreamOutput* stream);
    void restore(StreamOutput* stream, bool restore_position);
    static PrinterStateSnapshot capture();
    void invalidate();
    bool is_valid();

    // This constant is set when the current state can't be captured
    static PrinterStateSnapshot null_state;
private:
    PrinterStateSnapshot();
    PrinterStateSnapshot(float e, float x, float y, float z, float f, float s, bool r_abs, bool e_abs);
    float e, x, y, z, f, s;
    bool r_abs, e_abs;
    bool valid;
};

#endif // PRINTER_STATE_SNAPSHOT
