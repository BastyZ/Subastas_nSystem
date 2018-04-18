typedef struct subasta *Subasta;

Subasta nuevaSubasta(int unidades);
int ofrecer(Subasta s, double precio);
double adjudicar(Subasta s, int *punidades);
