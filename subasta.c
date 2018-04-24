#include <nSystem.h>
#include "subasta.h"

int adjudicado = 1, afuera = 0; // Adjudicado: Logra quedarse con producto

/*
typedef struct postor{
    int estado;
    double precio;
    nCondition cond;
} *Postor;
*/

typedef struct subasta {
  // ... implemente aca el tipo Subasta ...
  // (observe que hay un "typedef struct subasta *Subasta" en subasta.h)
    int finalizado;
    int unidades;
    int count; // cuenta de objetos adjudicados
    nMonitor monitor;
    double min;
    int indexMin;
    //Postor *postor; // Lista de postores que se adjudicarían unidades
    int *estado;
    double *precio;
    nCondition *cond;
}*Subasta;

// Programe aca las funciones nuevaSubasta, ofrecer y adjudicar, mas
// otras funciones que necesite.

Subasta nuevaSubasta(int unidades) {
    Subasta subasta = (Subasta)nMalloc(sizeof(*subasta));
    subasta->monitor = nMakeMonitor();
    subasta->finalizado = 0;
    subasta->unidades = unidades;
    subasta->count = 0;
    subasta->indexMin = 0;
    // subasta->postor = (Postor*)nMalloc(unidades*sizeof(Postor));
    subasta->estado = (int)nMalloc(unidades* sizeof(int));
    subasta->precio = (double)nMalloc(unidades* sizeof(double));
    subasta->cond = (nCondition)nMalloc(unidades* sizeof(nCondition));
    return subasta;
}

void agregarPostor(int index, double precio, Subasta s) {
    s->estado[index] = afuera;
    s->precio[index] = precio;
    p->cond[index] = nMakeCondition(s->monitor);
}

int comparaPrecio(Subasta s, double precio) {
    // Devuelve 0 si no adjudica elementos, 1 si lo hace
    if (s->min <= precio) {
        return 0;
    } else {
        return 1;
    }
}

int ofrecer(Subasta s, double precio){
    // oferta un precio con la intencion de comprar elementos
    // Subasta s y Postor p
    // Postor p = nuevoPostor(s->postor[s->count], precio);
    int indice;
    if (s->finalizado) {
        // La subasta ha terminado, por lo que no lo intentamos
        return 0;
    } else {
        nEnter(s->monitor); // Entra al monitor
        // La subasta sigue activa
        if (s->count == 0) { // Primer oferente, entramos al toque
            nPrintf("Soy el primer oferente\n");
            agregarPostor(s->count, precio, s);
            s->estado[s->count] = adjudicado;
            nPrintf("Le cambio el estado al nuevo postor\n");
            indice = s->count;
            nPrintf("  Creo un 'p' auxiliar\n");
            s->count++;
            s->min = precio;
            s->indexMin = 0; // redundante
            nPrintf("Me voy a dormir\n");
            nWaitCondition(s->cond[indice]);
            nPrintf("-- Desperte 1\n");
            if (s->finalizado) return 0; // No ha terminado aún nos sacaron
        } else {
            if (s->count < s->unidades) { // primeros n oferentes
                nPrintf("No soy el primer oferente\n");
                agregarPostor(s->count, precio, s);
                s->estado[s->count] = adjudicado;
                indice = s->count;
                s->count++;
                nPrintf("    Entré a la subasta\n");
                // se vuelve a establecer el mínimo
                for (int i = 0; i < s->count; i++){
                    nPrintf("    Comienzo a buscar el menor\n");
                    if( s->precio[i] < s->min){
                        nPrintf("    Los cambio\n");
                        s->min = s->precio[i];
                        s->indexMin = i;
                    }
                    nPrintf("    Y listo\n");
                }
                nWaitCondition(s->cond[indice]);
                nPrintf("-- Desperte 2\n");
                if (s->finalizado) return 0; // No ha terminado aún nos sacaron
            } else {
                nPrintf("la subasta está llena\n");
                if (comparaPrecio(s, precio)) {
                    // La oferta es muy pequeña, se rechaza
                    return 0;
                } else {
                    nPrintf("  La comparación salió bien\n");
                    // Hecho al minimo del arreglo
                    s->estado[s->indexMin] = afuera;
                    nPrintf("  Llamo al quese va\n");
                    nSignalCondition(s->cond[s->indexMin]);
                    nPrintf("  Llamé al wn que se vá\n");
                    // se adjudica un elemento poniendose en el lugar del minimo
                    agregarPostor(s->indexMin, precio, s);
                    nPrintf("  Me agrego\n");
                    s->estado[s->indexMin] = adjudicado;
                    nPrintf("Modifico el indice\n");
                    indice = s->count;
                    s->count++;
                    // volvemos e elegir el mínimo
                    for (int i = 0; i < s->unidades; i++){
                        nPrintf("    Comienzo a buscar el menor\n");
                        if( s->precio[i] < s->min){
                            nPrintf("    Los cambio\n");
                            s->min = s->precio[i];
                            s->indexMin = i;
                        }
                        nPrintf("    Y listo\n");
                    }
                    nWaitCondition(s->cond[indice]);
                    nPrintf("-- Desperte 3\n");
                    if (s->finalizado) return 0; // No ha terminado aún nos sacaron
                }
            }
        }
        // Si llegan hasta acá, o termino la subasta o los sacaron
        if (s->finalizado) return 0; // No ha terminado aún nos sacaron
        return 1;
    }
}

double colecta(Postor p) {
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
