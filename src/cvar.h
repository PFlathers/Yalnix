/*
 */
typedef struct _CVAR
{
        int id; //id of the cvar
        int claimed; // 0 if not claimed. 1 if claimed
        struct _Node *waiters; // who is waiting on signal.
} __CVAR;
typedef struct _CVAR CVAR;
