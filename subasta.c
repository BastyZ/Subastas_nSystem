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
    nPrintf("  Agrego a un este wn a la subasta\n");
    p->estado = afuera;
    nPrintf("  Estado agregado\n");
    p->precio = precio;
    p->listo = 0;
    nPrintf("  Falta solo la condicion\n");
    p->cond = nMakeCondition(s->monitor);
    nPrintf("  Termino de agregar a este wn\n");
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
            nPrintf("le paso algo del tipo %d\n", s->postor[s->count]);
            agregarPostor(&s->postor[s->count], precio, s);
            nPrintf("Agregue al postor\n");
            Postor p = &s->postor[s->count];
            p->estado = adjudicado;
            nPrintf("Lo agrego denuevo\n");
            s->postor[s->count]->estado = adjudicado;
            nPrintf("            hi!\n");
            nPrintf("Le cambio el estado al nuevo postor\n");
            indice = s->count;
            nPrintf("  Creo un 'p' auxiliar\n");
            Postor p = s->postor[s->count];
            s->count++;
            s->min = precio;
            s->indexMin = 0; // redundante
            nPrintf("Me voy a dormir\n");
            while (!s->finalizado && p->estado == adjudicado) {
                nWaitCondition(p->cond);
            }
        } else {
            if (s->count < s->unidades) { // primeros n oferentes
                nPrintf("No soy el primer oferente\n");
                agregarPostor(&s->postor[s->count], precio, s);
                s->postor[s->count]->estado = adjudicado;
                Postor p = s->postor[s->count];
                indice = s->count;
                s->count++;
                nPrintf("    Entré a la subasta\n");
                // se vuelve a establecer el mínimo
                for (int i = 0; i < s->count; i++){
                    nPrintf("    Comienzo a buscar el menor\n");
                    if( s->postor[i]->precio < s->min){
                        nPrintf("    Los cambio\n");
                        s->min = s->postor[i]->precio;
                        s->indexMin = i;
                    }
                    nPrintf("    Y listo\n");
                }
                while (!s->finalizado && p->estado == adjudicado) {
                    nWaitCondition(p->cond);
                }
            } else {
                nPrintf("la subasta está llena\n");
                if (comparaPrecio(s, precio)) {
                    // La oferta es muy pequeña, se rechaza
                    return 0;
                } else {
                    // Hecho al minimo del arreglo
                    s->postor[s->indexMin]->estado = afuera;
                    nSignalCondition(&s->postor[s->indexMin]->cond);
                    // se adjudica un elemento poniendose en el lugar del minimo
                    agregarPostor(s->postor[s->indexMin], precio, s);
                    s->postor[s->indexMin]->estado = adjudicado;
                    Postor p = s->postor[s->indexMin];
                    indice = s->count;
                    s->count++;
                    // volvemos e elegir el mínimo
                    for (int i = 0; i < s->count; i++) {
                        nPrintf("    Comienzo a buscar el menor\n");
                        if( s->postor[i]->precio < s->min){
                            nPrintf("    Los cambio\n");
                            s->min = s->postor[i]->precio;
                            s->indexMin = i;
                        }
                    }
                    while (!s->finalizado && p->estado == adjudicado) {
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
