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
    Postor *postor; // Lista de postores que se adjudicarían unidades
}*Subasta;

// Programe aca las funciones nuevaSubasta, ofrecer y adjudicar, mas
// otras funciones que necesite.

Subasta nuevaSubasta(int unidades) {
    Subasta subasta = nMalloc(sizeof(*subasta));
    subasta->monitor = nMakeMonitor();
    subasta->finalizado = 0;
    subasta->unidades = unidades;
    subasta->count = 0;
    subasta->indexMin = 0;
    subasta->postor = nMalloc(unidades*sizeof(Postor));
    return subasta;
}

void agregarPostor(Postor p, double precio, Subasta s) {
    p->estado = afuera;
    p->precio = precio;
    p->listo = 0;
    p->cond = nMakeCondition(s->monitor);
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
            agregarPostor(&s->postor[s->count], precio, s);
            Postor p = &s->postor[s->count];
            p->estado = adjudicado;
            nPrintf("Le cambio el estado al nuevo postor\n");
            indice = s->count;
            nPrintf("  Creo un 'p' auxiliar\n");
            s->count++;
            s->min = precio;
            s->indexMin = 0; // redundante
            nPrintf("Me voy a dormir\n");
            while (!s->finalizado && p->estado == adjudicado) {
                nPrintf("-- Desperte 1\n");
                nWaitCondition(p->cond);
            }
        } else {
            if (s->count < s->unidades) { // primeros n oferentes
                nPrintf("No soy el primer oferente\n");
                agregarPostor(&s->postor[s->count], precio, s);
                Postor p = &s->postor[s->count];
                p->estado = adjudicado;
                indice = s->count;
                s->count++;
                nPrintf("    Entré a la subasta\n");
                // se vuelve a establecer el mínimo
                for (int i = 0; i < s->count; i++){
                    nPrintf("    Comienzo a buscar el menor\n");
                    Postor aux = &s->postor[i];
                    if( aux->precio < s->min){
                        nPrintf("    Los cambio\n");
                        s->min = aux->precio;
                        s->indexMin = i;
                    }
                    nPrintf("    Y listo\n");
                }
                while (!s->finalizado && p->estado == adjudicado) {
                    nPrintf("-- Desperte 2\n");
                    nWaitCondition(p->cond);
                }
            } else {
                nPrintf("la subasta está llena\n");
                if (comparaPrecio(s, precio)) {
                    // La oferta es muy pequeña, se rechaza
                    return 0;
                } else {
                    nPrintf("  La comparación salió bien\n");
                    // Hecho al minimo del arreglo
                    Postor p = &s->postor[s->indexMin];
                    p->estado = afuera;
                    nSignalCondition(p->cond);
                    // se adjudica un elemento poniendose en el lugar del minimo
                    agregarPostor(s->postor[s->indexMin], precio, s);
                    s->postor[s->indexMin]->estado = adjudicado;
                    indice = s->count;
                    s->count++;
                    // volvemos e elegir el mínimo
                    for (int i = 0; i < s->count; i++){
                        nPrintf("    Comienzo a buscar el menor\n");
                        Postor aux = &s->postor[i];
                        if( aux->precio < s->min){
                            nPrintf("    Los cambio\n");
                            s->min = aux->precio;
                            s->indexMin = i;
                        }
                        nPrintf("    Y listo\n");
                    }
                    while (!s->finalizado && p->estado == adjudicado) {
                        nPrintf("-- Desperte 3\n");
                        nWaitCondition(p->cond);
                    }
                }
            }
        }
        // Si llegan hasta acá, o termino la subasta o los sacaron
        if (!s->finalizado) return 0; // No ha terminado aún nos sacaron
        return 0;
    }
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
