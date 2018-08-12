extern bool isFDS;

// Pointer to base disk data for interfacing with RA
extern uint8 *FDSROM;
extern uint32 FDSSize;

void FDSSoundReset(void);

void FCEU_FDSInsert(void);
//void FCEU_FDSEject(void);
void FCEU_FDSSelect(void);
