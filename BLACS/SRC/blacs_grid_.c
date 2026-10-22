#include "Bdef.h"

#if (INTFACE == C_CALL)
void Cblacs_gridexit(Int ConTxt)
#else
F_VOID_FUNC blacs_gridexit_(Int *ConTxt)
#endif
{
   Int i;
   BLACSCONTEXT *ctxt;
   extern Int BI_MaxNCtxt;
   extern BLACSCONTEXT **BI_MyContxts;

   if ( (Mpval(ConTxt) < 0) || (Mpval(ConTxt) >= BI_MaxNCtxt) )
      BI_BlacsErr(Mpval(ConTxt), __LINE__, __FILE__,
 	          "Trying to exit non-existent context");

   if (BI_MyContxts[Mpval(ConTxt)] == NULL)
      BI_BlacsErr(Mpval(ConTxt), __LINE__, __FILE__,
	          "Trying to exit an already freed context");

   MGetConTxt(Mpval(ConTxt), ctxt);
/* Wait for this context's pending asynchronous operations: the lazy
   sends only complete when a later BLACS call tests them, so without
   this wait the communicator stays alive past MPI_Finalize as a leak. */
   {
      void BI_UpdateBuffs(BLACBUFF *);
      extern BLACBUFF *BI_ActiveQ;
      BLACBUFF *bp;
      int mine;
      do {
         BI_UpdateBuffs(NULL);
         mine = 0;
         for (bp = BI_ActiveQ; bp; bp = bp->next)
            if (bp->ctxt == ctxt) { mine = 1; break; }
      } while (mine);
   }
/*
 * Destroy context
 */
   MPI_Comm_free(&ctxt->pscp.comm);
   MPI_Comm_free(&ctxt->ascp.comm);
   MPI_Comm_free(&ctxt->rscp.comm);
   MPI_Comm_free(&ctxt->cscp.comm);
   free(ctxt);
   BI_MyContxts[Mpval(ConTxt)] = NULL;
}
