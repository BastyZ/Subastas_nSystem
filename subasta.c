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

Subasta nuevaSubasta(int unidades) {
    Subasta subasta = nMalloc(sizeof(*subasta));
    subasta->monitor = nMakeMonitor();
    subasta->finalizado = 0;
    subasta->unidades = unidades;
    subasta->count = 0;
    subasta->indexMin = 0;
    Postor postor1 = nMalloc(sizeof(*postor1));
    subasta->postor = nMalloc(unidades*sizeof(*postor1));
    return subasta;
}

Postor nuevoPostor(Subasta s, int precio) {
    Postor p = nMalloc(sizeof(*p));
    p->estado = afuera;
    p->precio = precio;
    p->listo = 0;
    p->cond = nMakeCondition(s->monitor);
    return p;
}

int comparaPrecio(Subasta s, Postor p) {
    // Devuelve 0 si no adjudica elementos, 1 si lo hace
    if (s->min <= p->precio) {
        return 0;
    } else {
        return 1;
    }
}

void cambiar(Subasta s, Postor p) {
    // Tomamos al más pequeño, lo sacamos y reemplazamos en ese lugar
    nPrintf("Entro a cambiar\n");
    Postor aux = s->postor[s->indexMin];
    aux->estado = afuera;
    aux->listo = 1;
    // notifico al que saco de que lo saqué
    nSignalCondition(aux->cond);
    p->estado = adjudicado;
    s->postor[s->indexMin] = p;
}

int ofrecer(Subasta s, double precio){
    // oferta un precio con la intencion de comprar elementos
    // Subasta s y Postor p
    Postor p = nuevoPostor(s, precio);
    if (s->finalizado) {
        // La subasta ha terminado, por lo que no lo intentamos
        return 0;
    } else {
        nEnter(s->monitor); // Entra al monitor
        // La subasta sigue activa
        if (s->count == 0) { // Primer oferente, entramos al toque
            nPrintf("Soy el primer oferente\n");
            p->estado = adjudicado;
            s->postor[s->count++] = p;
            s->min;
            while (!s->finalizado && p->estado == adjudicado) {
                nWaitCondition(p->cond);
            }
        } else {
            if (s->count < s->unidades) { // primeros n oferentes
                nPrintf("No soy el primer oferente\n");
                p->estado = adjudicado;
                s->postor[s->count++] = &p;
                nPrintf("    Entré a la subasta\n");
                // se vuelve a establecer el mínimo
                for (int i = 0; i < s->count; i++){
                    nPrintf("    Comienzo a buscar el menor\n");
                    if( *(s->postor[s->count])->precio < s->min){
                        nPrintf("    Los cambio\n");
                        s->min = *(s->postor[s->count])->precio;
                        s->count++;
                        s->indexMin = i;
                    }
                    nPrintf("    Y listo\n");
                }
                while (!s->finalizado && p->estado == adjudicado) {
                    nWaitCondition(p->cond);
                }
            } else {
                nPrintf("la subasta está llena\n");
                if (comparaPrecio(s, p)) {
                    // La oferta es muy pequeña, se rechaza
                    return 0;
                } else {
                    // se adjudica un elemento
                    p->estado = adjudicado;
                    cambiar(s, p);
                    // volvemos e elegir el mínimo
                    for (int i = 0; i < s->count; i++) {
                        Postor aux = s->postor[i];
                        if(aux->precio < s->min){
                            s->min = aux->precio;
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
        // Terminó y adjudica elemento
        if (s->finalizado && p->estado == adjudicado) return 1;
        // Acá nos aseguramos de que si nos echaron retornamos 0
        if (p->listo && p->estado == afuera) return 0;
        // Para lo demás rechazamos
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
