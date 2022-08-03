#include <jsonparser.h>

/*
 * estructuras
 */

struct faro{
	unsigned short posicion[2];
	short dueno;
	unsigned int energia;
	unsigned short llave;
	unsigned int **distancia;
	unsigned short *conexiones;
	unsigned short n_conexiones;
};

struct triangulo{
	unsigned int faros[3];
	unsigned int puntos;
	unsigned int perimetro;
	unsigned int energia;
};

struct game{
	unsigned short turno;
	short player_num;
	unsigned short player_count;
	unsigned short **map;
	unsigned short maxy;
	unsigned short maxx;
	unsigned short faros_count;
	unsigned short tri_count;
	struct faro *faros;
	struct triangulo *triangulos;
};

struct player{
	unsigned short posicion[2];
	unsigned int energia;
	unsigned int **view;
	unsigned short v_maxx;
	unsigned short v_maxy;

};

struct objetivo{
	unsigned short next;
	unsigned short connect;
	unsigned short energy;
	unsigned short n_energy;
	unsigned short tri;
};

/*
 * Funciones de Utilidad
 */
short parse_game(struct game *juego, struct object root);
short parse_turn(struct player *jugador, struct game *juego, struct object root);
unsigned short get_lightid(unsigned short y, unsigned short x,struct game *juego);
short is_conected(unsigned short v,struct faro f);
struct objetivo get_objetive(struct player jugador, struct game juego);
short check_tri(struct game juego, unsigned int t_turn, unsigned int n_turn, unsigned short t_tri, unsigned short n_tri);
short check_tri_cross(struct game juego,unsigned short tri);
short check_cross(struct game juego, unsigned short f1, unsigned short f2);
short do_intersect(unsigned short p1[2],unsigned short q1[2],unsigned short p2[2],unsigned short q2[2]);
short orientation(unsigned short p[2],unsigned short q[2], unsigned short r[2]);
short on_segment(unsigned short q1[2], unsigned short q2[2], unsigned short p[2]);
short is_inside(unsigned short t1[2], unsigned short t2[2], unsigned short t3[2], unsigned short p[2]);
int triangle_point(unsigned short t1[2], unsigned short t2[2], unsigned short t3[2],struct game *juego);
unsigned int **flood_dist(unsigned short **map,unsigned short maxx, unsigned short maxy, unsigned short pos[2]);
short check_in(int cell[2],int **checked, unsigned int len);

/*
 * funciones de comunicaciones
 */
void snd_log(char *msg);
size_t read(struct object *root);
void attack(unsigned int ener);
void connect(unsigned short x, unsigned short y);
void move(short x, short y);
