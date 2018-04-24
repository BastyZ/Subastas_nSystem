#include "nSystem.h"
#include "subasta.h"

// Para poder usar rand
#include <stdlib.h>

nSem mutex;

// print_msg:
//   1   => con mensajes y delay
//   0   => sin mensajes, con delay
//   -1  => sin mensajes, sin delay

int aleatorio(Subasta s, int print_msg, char *nom, int oferta) {
  if (print_msg>=0) {
    nSetTaskName(nom);
    nWaitSem(mutex); 
    int delay= rand()%1000;
    nSignalSem(mutex);
    nSleep(delay);
  }
  if (print_msg>0)
    nPrintf("%s ofrece %d\n", nom, oferta);
  int ret= ofrecer(s, oferta);
  if (print_msg>0) {
    if (ret)
      nPrintf("%s se adjudico un item a %d\n", nom, oferta);
    else
      nPrintf("%s fallo con su oferta de %d\n", nom, oferta);
  }
  return ret;
}

int oferente(Subasta s, int print_msg, char *nom, int oferta) {
  nSetTaskName(nom);
  if (print_msg>0)
    nPrintf("%s ofrece %d\n", nom, oferta);
  int ret= ofrecer(s, oferta);
  if (print_msg>0) {
    if (ret)
      nPrintf("%s se adjudico un item a %d\n", nom, oferta);
    else
      nPrintf("%s fallo con su oferta de %d\n", nom, oferta);
  }
  return ret;
}

int test1(int print_msg) {
  nPrintf("inicio test 1");
  Subasta s= nuevaSubasta(0);
  nTask pedro= nEmitTask(aleatorio, s, print_msg, "pedro", 1);
  nTask juan= nEmitTask(aleatorio, s, print_msg, "juan", 3);
  nTask diego= nEmitTask(aleatorio, s, print_msg, "diego", 4);
  nTask pepe= nEmitTask(aleatorio, s, print_msg, "pepe", 2);
  if (nWaitTask(pedro))
    nFatalError("test1", "pedro debio perder con 1\n");
  if (nWaitTask(pepe))
    nFatalError("test1", "pepe debio perder con 2\n");
  int u;
  int recaud= adjudicar(s, &u);
  if (recaud!=0)
    nFatalError("test1", "La recaudacion debio ser 0 y no %d\n", recaud);
  if (u!=0)
    nFatalError("test1", "Quedaron %d unidades sin vender\n", u);
  if (nWaitTask(juan))
    nFatalError("nMain", "juan debio perder con 6\n");
  if (nWaitTask(diego))
    nFatalError("nMain", "diego debio perder con 4\n");
  if (print_msg>0)
    nPrintf("El monto recaudado es %d y quedaron %d unidades sin vender\n",
            recaud, u);
  return 0;
}

int test2(int print_msg) {
  Subasta s= nuevaSubasta(3);
  nTask ana= nEmitTask(oferente, s, print_msg, "ana", 7);
  if (print_msg>=0)
    nSleep(1000);
  nTask maria= nEmitTask(oferente, s, print_msg, "maria", 3);
  if (print_msg>=0)
    nSleep(1000);
  nTask ximena= nEmitTask(oferente, s, print_msg, "ximena", 4);
  if (print_msg>=0)
    nSleep(1000);
  nTask erika= nEmitTask(oferente, s, print_msg, "erika", 5);
  if (print_msg>=0)
    nSleep(1000);
  if (nWaitTask(maria))
    nFatalError("nMain", "maria debio perder con 3\n");
  nTask sonia= nEmitTask(oferente, s, print_msg, "sonia", 6);
  if (print_msg>=0)
    nSleep(1000);
  if (nWaitTask(ximena))
    nFatalError("nMain", "ximena debio perder con 4\n");
  int u;
  int recaud= adjudicar(s, &u);
  if (recaud!=18)
    nFatalError("test1", "La recaudacion debio ser 7 y no %d\n", recaud);
  if (u!=0)
    nFatalError("test1", "Quedaron %d unidades sin vender\n", u);
  if (!nWaitTask(ana))
    nFatalError("nMain", "ana debio ganar con 7\n");
  if (!nWaitTask(erika))
    nFatalError("nMain", "erika debio ganar con 5\n");
  if (!nWaitTask(sonia))
    nFatalError("nMain", "sonia debio ganar con 6\n");
  if (print_msg>0)
    nPrintf("El monto recaudado es %d y quedaron %d unidades sin vender\n",
            recaud, u);
  return 0;
}


#define N 30
#define M 20000

int nMain() {
  mutex= nMakeSem(1);
  nPrintf("una sola subasta con tiempos aleatorios\n");
  test1(1);
  nPrintf("test aprobado\n");
  nPrintf("-------------\n");

  nPrintf("una sola subasta con tiempos deterministicos\n");
  test2(1);
  nPrintf("test aprobado\n");
  nPrintf("-------------\n");

  nSetTimeSlice(1);
  nPrintf("%d subastas en paralelo\n", N);
  nTask *tasks1= nMalloc(M*sizeof(nTask));
  nTask *tasks2= nMalloc(M*sizeof(nTask));
  int k;
  for (k=1; k<N; k++) {
    tasks1[k]= nEmitTask(test1, 0);
    tasks2[k]= nEmitTask(test2, 0);
  }
  tasks1[0]= nEmitTask(test1, 1);
  tasks2[0]= nEmitTask(test2, 1);
  for (k=0; k<N; k++) {
    nWaitTask(tasks1[k]);
    nWaitTask(tasks2[k]);
  }
  nPrintf("test aprobado\n");
  nPrintf("-------------\n");
  nPrintf("%d subastas en paralelo\n", M*2);
  nPrintf("(tomo alrededor de 20 segundos en mi computador)\n");
  for (k=1; k<M; k++) {
    tasks1[k]= nEmitTask(test1, -1);
    tasks2[k]= nEmitTask(test2, -1);
  }
  tasks1[0]= nEmitTask(test1, 1);
  tasks2[0]= nEmitTask(test2, 1);
  nWaitTask(tasks1[0]);
  nWaitTask(tasks2[0]);
  nPrintf("Enterrando tareas.  Cada '.' son 30 tareas enterradas.\n");
  for (k=1; k<M; k++) {
    nWaitTask(tasks1[k]);
    nWaitTask(tasks2[k]);
    if (k%10==0) nPrintf(".");
  }
  nPrintf("\ntest aprobado\n");
  nPrintf(  "-------------\n");
  nPrintf("Felicitaciones: paso todos los tests\n");
  return 0;
}
