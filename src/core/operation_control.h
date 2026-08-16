#pragma once

// Arms and polls the common exit gesture used by long-running Sentinel tools.
// Two distinct presses of the top button within 550 ms stop the operation.
void armOperationExit();
bool operationExitRequested();
