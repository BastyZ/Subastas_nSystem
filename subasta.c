#include <nSystem.h>
#include "subasta.h"

typedef enum {
    adjudicado, afuera // Adjudicado: Logra quedarse con producto
} Estado;

typedef struct postor{
    Estado estado;
    double precio;
    int listo; // 0 => No está listo
    nCondition cond;
} *Postor;

typedef struct subasta {
  // ... implemente aca el tipo Subasta ...
  // (observe que hay un "typedef struct subasta *Subasta" en subasta.h)
    int finalizado;
    int unidades;
    int count; // cuenta de objetos adjudicados
    nMonitor monitor;
    double min;
    int indexMin;
    int *postor; // Lista de postores que se adjudicarían unidades
}*Subasta;

// Programe aca las funciones nuevaSubasta, ofrecer y adjudicar, mas
// otras funciones que necesite.

subasta nuevaSubasta(int unidades){
    Subasta subasta = (Subasta)nMalloc(sizeof(*subasta));
    subasta->monitor = nMakeMonitor();
    subasta->finalizado = 0;
    subasta->unidades = unidades;
    subasta->count = 0;
    subasta->indexMin = 0;
    subasta.postor = (Postor)nMalloc(unidades*sizeof(*Postor));
}

int ofrecer(Subasta s, double precio){
    // oferta un precio con la intencion de comprar elementos
}

double colecta(Postor p) {
    p->listo = 1;
    int precio = p->precio;
    nSignalCondition(p->cond);
    return precio;
}

double adjudicar(Subasta s, int *punidades){
    // *punidades == *prestantes
    nEnter(s->monitor);
    *punidades = s->unidades - s->count;
    s->finalizado = 1; // convertirlo a true
    int ganancia;
    for (int i=0; i < s->count; i++){
        ganancia += colecta(s->postor[i]);
    }
    nExit(s->monitor);
}
