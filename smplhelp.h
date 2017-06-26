/*
  Interface to Simple Help box
*/

VOID CALLBACK SimpleHelp (HAB hab, HWND hwndUser, PSZ achUser,
              HMODULE hmodUser, USHORT idType, USHORT idName);

VOID CALLBACK SimpleHelpExit (void);

VOID CALLBACK SimpleHelpInit (HAB hab);
