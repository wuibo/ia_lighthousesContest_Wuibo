#include <stdio.h>
#include <stdio_ext.h>
#include <string.h>
#include <stdlib.h>
#include <utils.h>
#include <math.h>

/*
 * CONSTANTES
 */
#define ST_NL 0 //Esperamos [
#define ST_NM 1 //esperamos numero, coma o ]
#define ST_IL 2 //fin de la linea

#define TRUE 1
#define FALSE 0

#define E_DIST 5
#define E_TURN 10
#define E_FRAC 2
#define E_LOSS 10
#define E_PROB 0.05


/*
 * Variables globales
 */

char name[9] = "WuiboEE30";
int MAX_CANON = 200;
char msg[200] ="tmp";

/*
 * funciones utiles
 */



short parse_game(struct game *juego, struct object root){
	double max_tri;
	unsigned short *temp_line;
	unsigned short *back_line;
	unsigned short **temp_map;
	unsigned short **back_map;
	char array_char;
	unsigned short maxy=0;
	unsigned short maxx;
	unsigned short light_count = 0;
	unsigned short act_num=0,act_x=0;
	struct object *p_temp,*map,*p_mline;
	struct object otemp,mline;
	struct faro *temp_faros;
	struct faro *back_faros;
	struct faro t_faro;
	struct triangulo *temp_tri;
	struct triangulo *back_tri;
	int j,id;
	double n=1,nr=1,i;
	unsigned int fst,snd,cnt,tri_count,points;
	unsigned int d12,d23,d31,perimeter;

	//inicializar turnos a 0
	juego->turno = 0;

	p_temp = root.content;
	//numero de jugador	
	id = json_get_item_by_name(root,"player_num");
	if(id<0){
		snd_log("no se encuentra player_num");
		return -1;
	}else{
		otemp = p_temp[id];
		juego->player_num = (short) json_get_number(otemp);
	}

	//cantidad de jugadores
	id = json_get_item_by_name(root,"player_count");
	if(id<0){
		snd_log("no se encuentra player_count");
		return -1;
	}else{
		otemp = p_temp[id];
		juego->player_count = (unsigned short) json_get_number(otemp);
	}	

	//mapa
	id= json_get_item_by_name(root,"map");
	if(id<0){
		snd_log("no se encuentra map");
		return -1;
	}
	otemp = p_temp[id];
	if(otemp.type != json_type_array || otemp.size <= 0){
		snd_log("tipo de mapa erroneo");
		return -1;
	}
	maxy = otemp.size;
	map = otemp.content;
	otemp = map[0];
	if(otemp.type != json_type_array || otemp.size <= 0){
		snd_log("tipo de linea de mapa erroneo");
		return -1;
	}else{
		maxx = otemp.size;
		juego->map = malloc(maxy*sizeof(unsigned short*));
		for(j=0;j<maxy;j++){
			otemp = map[j];
			if(otemp.type != json_type_array || otemp.size !=maxx){
				//todas las líneas del mismo tamaño
				snd_log("linea de mapa de distinto tamaño");
				return -1;
			}
			p_mline = otemp.content;
			temp_line = malloc(maxx*sizeof(unsigned short));
			for(i=0;i<maxx;i++){
				mline = p_mline[(int) i];
				temp_line[(int) i] = (unsigned short) json_get_number(mline);
			}
			juego->map[j] = temp_line;
		}
	}			
	juego->maxy = maxy;
	juego->maxx = maxx;
	

	//imprimir mapa
/*	
	for(j=(maxy-1);j>=0;j--){
		for(act_x=0;act_x<maxx;act_x++){
			if(juego->map[j][act_x]==1){
				fprintf(stderr," ");
			}else{
				fprintf(stderr,"#");
			}
		}
		fprintf(stderr,"\n");
	}

*/
	//cargar faros
	id = json_get_item_by_name(root,"lighthouses");
	if(id<0){
		snd_log("no se encuentra lighthouses");
		return -1;
	}
	otemp = p_temp[id];
	if(otemp.type != json_type_array || otemp.size <= 0){
		snd_log("tipo de faros erroneo");
		return -1;
	}
	juego->faros_count = otemp.size;
	juego->faros = malloc(juego->faros_count*sizeof(struct faro));
	p_mline = otemp.content;
	for(i=0;i<juego->faros_count;i++){
		mline = p_mline[(int) i];
		if(mline.type != json_type_array || mline.size != 2){
			snd_log("definición de faros erronea");
			return -1;
		}
		p_temp = mline.content;
		juego->faros[(int) i].posicion[1] = json_get_number(p_temp[0]);
		juego->faros[(int) i].posicion[0] = json_get_number(p_temp[1]);
		juego->faros[(int) i].dueno = -1;
		juego->faros[(int) i].energia = 0;
		juego->faros[(int) i].llave = 0;

		//mapa de distancias
		juego->faros[(int) i].distancia = flood_dist(juego->map,juego->maxx,juego->maxy,juego->faros[(int) i].posicion);
		
	}

	//imprimir distancias de faro x
/*
	fprintf(stderr,"Faro 1: [%u,%u]\n",juego->faros[1].posicion[0],juego->faros[1].posicion[1]);
	unsigned int d_val;
	for(j=(maxy-1);j>=0;j--){
		for(act_x=0;act_x<maxx;act_x++){
			d_val = juego->faros[1].distancia[j][act_x];
			if(d_val<10){
				fprintf(stderr,"%u   ",d_val);
			}else{
				if(d_val<100){
					fprintf(stderr,"%u  ",d_val);
				}else{
					if(d_val<1000){
						fprintf(stderr,"%u ",d_val);
					}else{						
						fprintf(stderr,"%u",d_val);
					}
				}
			}
		}
		fprintf(stderr,"\n");
	}
	fprintf(stderr,"Fin de imprimir distancia\n");
*/	
	
	//imprimier faros
/*
	fprintf(stderr,"imprimir faros: %d\n",juego->faros_count);
	for(j=0;j<juego->faros_count;j++){
		snprintf(msg,MAX_CANON,"[%d,%d]\n",juego->faros[j].posicion[0],juego->faros[j].posicion[1]);
		snd_log(msg);
	}
*/
		

	//cargar triangulos
	//comprobar cantidad
	for(n=1,i=1;i<=juego->faros_count;i++){
		n=n*i;
	}
	for(nr=1,i=1;i<=(juego->faros_count-3);i++){
		nr=nr*i;
	}
	max_tri = n/(6*nr);

	tri_count = 0;
	fst = 0;
	snd = 1;
	cnt = 2;
	//snprintf(msg,MAX_CANON,"max_tri %lf, n: %lf nr: %lf", max_tri, n, nr);
	//snd_log(msg);
	for(i=0;i<max_tri;i++){
		points = triangle_point(juego->faros[fst].posicion,juego->faros[snd].posicion,
			juego->faros[cnt].posicion,juego);
		//snprintf(msg,MAX_CANON,"tri: %lf, points: %u", i, points);
		//snd_log(msg);
		if(points>0 && orientation(juego->faros[fst].posicion,juego->faros[snd].posicion,
			juego->faros[cnt].posicion) != 0){
			//triangulo válido
			//nuevo triangulo
			back_tri = temp_tri;
			temp_tri = malloc((tri_count+1)*sizeof(struct triangulo));
			if(tri_count>0){
				for(j=0;j<tri_count;j++){
					temp_tri[j] = back_tri[j];
				}
				free(back_tri);
			}
			d12 = juego->faros[fst].distancia[juego->faros[snd].posicion[0]][juego->faros[snd].posicion[1]];
			d23 = juego->faros[snd].distancia[juego->faros[cnt].posicion[0]][juego->faros[cnt].posicion[1]];
			d31 = juego->faros[cnt].distancia[juego->faros[fst].posicion[0]][juego->faros[fst].posicion[1]];
			temp_tri[tri_count].perimetro = d12+d23+d31;
			temp_tri[tri_count].puntos = points;
			temp_tri[tri_count].energia = (temp_tri[tri_count].perimetro + E_TURN + temp_tri[tri_count].perimetro/E_FRAC)*E_LOSS;
			temp_tri[tri_count].faros[0] = fst;
			temp_tri[tri_count].faros[1] = snd;
			temp_tri[tri_count].faros[2] = cnt;
			tri_count++;
		}
		//avanzar la cuenta
		cnt++;
		if(cnt >= juego->faros_count){
			snd++;
			if(snd >= juego->faros_count-1){
				fst++;
				snd = fst+1;
			}
			cnt = snd +1;
		}
	}
	juego->triangulos = temp_tri;
	juego->tri_count = tri_count;
	return 0;	

}

short parse_turn(struct player *jugador, struct game * juego,  struct object root){
	int id;
	struct object *p_temp,*p_cline,*p_mline,*f_temp,*map;
	struct object otemp,mline,ftemp;
	unsigned short maxy,maxx;
	unsigned short i,j,idf;
	unsigned int *temp_line;
	
	//posicion
	id = json_get_item_by_name(root,"position");
	if(id<0){
		snd_log("no se encuentra posición");
	}else{
		p_temp = root.content;
		otemp = p_temp[id];
		if(otemp.type != json_type_array || otemp.size != 2){
			snd_log("tipo de posición erroneo");
		}else{
			p_temp = otemp.content;
			otemp = p_temp[0];
			jugador->posicion[1] = (short) json_get_number(otemp);
			otemp = p_temp[1];
			jugador->posicion[0] = (short) json_get_number(otemp);
		}
	}

	//energia
	id = json_get_item_by_name(root,"energy");
	if(id<0){
		snd_log("no se encuentra energia");
	}
	p_temp = root.content;
	otemp = p_temp[id];
	jugador->energia = (unsigned int) json_get_number(otemp);

	//view
	if(juego->turno > 0){
		for(i=0;i<jugador->v_maxy;i++){
			free(jugador->view[i]);
		}
		free(jugador->view);
	}

	id= json_get_item_by_name(root,"view");
	if(id<0){
		snd_log("no se encuentra view");
	}
	otemp = p_temp[id];
	if(otemp.type != json_type_array || otemp.size <= 0){
		snd_log("tipo de view erroneo");
	}
	maxy = otemp.size;
	map = otemp.content;
	otemp = map[0];
	if(otemp.type != json_type_array || otemp.size <= 0){
		snd_log("tipo de view erroneo");
		return -1;
	}else{
		maxx = otemp.size;
		jugador->view = malloc(maxy*sizeof(unsigned short*));
		for(j=0;j<maxy;j++){
			otemp = map[j];
			if(otemp.type != json_type_array || otemp.size !=maxx){
				//todas las líneas del mismo tamaño
				return -1;
			}
			p_mline = otemp.content;
			temp_line = malloc(maxx*sizeof(unsigned int));
			for(i=0;i<maxx;i++){
				mline = p_mline[i];
				temp_line[i] = (unsigned int) json_get_number(mline);
			}
			jugador->view[j] = temp_line;
		}
	}
	jugador->v_maxx = maxx;
	jugador->v_maxy = maxy;

	//actualizar faros
	id = json_get_item_by_name(root,"lighthouses");
	if(id<0){
		snd_log("lighthouses no encontrado");
		return -1;
	}
	p_temp = root.content;
	otemp = p_temp[id];
	if(otemp.type != json_type_array){
		snd_log("tipo lighthouses erroneo");
		return -1;
	}
	p_mline = otemp.content;
	for(i=0;i<otemp.size;i++){
		mline = p_mline[i];
		if(mline.type != json_type_object){
			snd_log("tipo de faro erroneo");
			return -1;
		}
		f_temp = mline.content;
		//identificar faro
		id = json_get_item_by_name(mline,"position");
		if(id<0){
			snd_log("position del faro no encontrada");
			return -1;
		}
		ftemp = f_temp[id];
		p_temp = ftemp.content;
		idf = get_lightid((unsigned short)json_get_number(p_temp[1]),(unsigned short)json_get_number(p_temp[0]),juego);
		//actualizar dueño
		id = json_get_item_by_name(mline,"owner");
		if(id<0){
			//PRUEBA
			ftemp = f_temp[0];
			/*
			snprintf(msg,MAX_CANON,"primer valor: %s",ftemp.name);
			snd_log(msg);
			snprintf(msg,MAX_CANON,"faro owner no encontrado %hu",i);
			snd_log(msg);	
			*/
			return -1;
		}
		ftemp = f_temp[id];
		if(ftemp.type != json_type_number){
			if(ftemp.type == json_type_null){
				juego->faros[idf].dueno = -1;
			}else{
				snd_log("error en tipo de dueño");
				return -1;
			}
		}else{
			juego->faros[idf].dueno = (short) json_get_number(ftemp);
		}
		//actualizar energia
		id = json_get_item_by_name(mline,"energy");
		if(id<0){
			snd_log("faro energy no encontrado");
			return -1;
		}
		ftemp = f_temp[id];
		juego->faros[idf].energia = (unsigned int) json_get_number(ftemp);
		//actualizar llave
		id = json_get_item_by_name(mline,"have_key");
		if(id<0){
			snd_log("faro have_key no encontrado");
			return -1;
		}
		ftemp = f_temp[id];
		if(json_get_boolean(ftemp) == 0){
			juego->faros[idf].llave = FALSE;
		}else{
			juego->faros[idf].llave = TRUE;
		}
		//conexiones
		id = json_get_item_by_name(mline,"connections");
		if(id<0){
			snd_log("faro connections no encontrado");
			return -1;
		}
		ftemp = f_temp[id];
		p_cline = ftemp.content;
		if(juego->turno >0){
			free(juego->faros[idf].conexiones);
		}
		juego->faros[idf].conexiones = malloc(ftemp.size*sizeof(unsigned short));
		for(j=0;j<ftemp.size;j++){
			mline = p_cline[j];
			p_temp = mline.content;
			juego->faros[idf].conexiones[j] = get_lightid((unsigned short) json_get_number(p_temp[1]),(unsigned short) json_get_number(p_temp[0]),juego);
		}
		juego->faros[idf].n_conexiones = ftemp.size;
		//snprintf(msg,MAX_CANON,"faros %hu posicion[%hu,%hu], dueño: %hd, llave: %hu",idf,juego->faros[idf].posicion[1],juego->faros[idf].posicion[0], juego->faros[idf].dueno, juego->faros[idf].llave);
		//snd_log(msg);
		
		
	}
	juego->turno++;

	return 0;
	
}

struct objetivo get_objetive(struct player jugador, struct game juego){
	//devuelve el objetivo a realizar
	unsigned short to = juego.tri_count; //triangulo objetivo
	unsigned short t_tri = juego.tri_count; //triangulo temporal
	unsigned int t_turn = juego.maxx*juego.maxy;
	unsigned int f_min = juego.maxx*juego.maxy;
	unsigned int f_d = juego.maxx*juego.maxy;
	unsigned int td;
	unsigned int i,j;
	unsigned short controlados = 0;	
	unsigned int df0,df1,df2,d01,d12,d20,d0,d1,d2;
	unsigned int n_next,n_connect,n_turn,nxt_dist;
	unsigned int e0,e1,e2;
	unsigned short cnd[3];
	unsigned short otr[3];
	int ener,n_ener;
	struct objetivo obj;
	
	//distancia al faro mas cercano que no tengamos llave
	for(i=0;i<juego.faros_count;i++){
		td = juego.faros[i].distancia[jugador.posicion[0]][jugador.posicion[1]];
		if (td < f_min && juego.faros[i].llave == FALSE){
			f_min = td;
			f_d = i;
		}
	}
	//iniciarlizar objtivo por si no se puede más
	obj.next = f_d;
	obj.connect = juego.faros_count;
	obj.energy = 0;
	obj.n_energy = 0;
	obj.tri = t_tri;

	//recorrer triangulo
	for(i=0;i<juego.tri_count;i++){
		//comprobar cuantos controlo
		controlados = 0;
		if(juego.faros[juego.triangulos[i].faros[0]].dueno == juego.player_num){
			cnd[0] = juego.triangulos[i].faros[0];
			controlados++;
		}else{
			otr[0] = juego.triangulos[i].faros[0];
		}

		if(juego.faros[juego.triangulos[i].faros[1]].dueno == juego.player_num){
			cnd[controlados] = juego.triangulos[i].faros[1];
			controlados++;
		}else{
			otr[1-controlados] = juego.triangulos[i].faros[1];
		}
		if(juego.faros[juego.triangulos[i].faros[2]].dueno == juego.player_num){
			cnd[controlados] = juego.triangulos[i].faros[2];
			controlados++;
		}else{
			otr[2-controlados] = juego.triangulos[i].faros[2];
		}


		switch(controlados){
		case 0:
			n_connect = juego.faros_count;
			//controlo 0
			df0 = juego.faros[juego.triangulos[i].faros[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
			df1 = juego.faros[juego.triangulos[i].faros[1]].distancia[jugador.posicion[0]][jugador.posicion[1]];
			df2 = juego.faros[juego.triangulos[i].faros[2]].distancia[jugador.posicion[0]][jugador.posicion[1]];
			d01 = juego.faros[juego.triangulos[i].faros[0]].distancia[juego.faros[juego.triangulos[i].faros[1]].posicion[0]][juego.faros[juego.triangulos[i].faros[1]].posicion[1]];
			d12 = juego.faros[juego.triangulos[i].faros[1]].distancia[juego.faros[juego.triangulos[i].faros[2]].posicion[0]][juego.faros[juego.triangulos[i].faros[2]].posicion[1]];
			d20 = juego.faros[juego.triangulos[i].faros[2]].distancia[juego.faros[juego.triangulos[i].faros[0]].posicion[0]][juego.faros[juego.triangulos[i].faros[0]].posicion[1]];
			//minimo de 0
			d0 = fmin((df0+juego.triangulos[i].perimetro+6), fmin((df0+(2*d01)+d20+7),(df0+(2*d20)+d01+7)));
			//minimo de 1
			d1 = fmin((df1+juego.triangulos[i].perimetro+6), fmin((df1+(2*d01)+d12+7), (df1+(2*d12)+d01+7)));
			//minimo de 2
			d2 = fmin((df2+juego.triangulos[i].perimetro+6), fmin((df2+(2*d20)+d12+7), (df2+(2*d12)+d20+7)));
			//minimo de todos
			n_turn = fmin(d0,fmin(d1,d2));
			if(n_turn!=d1 && n_turn!= d2){
				//hacia el 0
				n_next = juego.triangulos[i].faros[0];
			}else if(n_turn!= d0 && n_turn != d2){
				//hacia el 1
				n_next = juego.triangulos[i].faros[1];
			}else if(n_turn != d0 && n_turn != d1){
				//hacie el 2
				n_next = juego.triangulos[i].faros[2];
			}else if (n_turn != d0){
				//d1 y d2 minimos
				if( juego.faros[juego.triangulos[i].faros[1]].energia <= juego.faros[juego.triangulos[i].faros[2]].energia){
					n_next = juego.triangulos[i].faros[1];
				}else{
					n_next = juego.triangulos[i].faros[2];
				}
			}else if (n_turn != d1){
				//d0 y d2 minimos
				if( juego.faros[juego.triangulos[i].faros[0]].energia <= juego.faros[juego.triangulos[i].faros[2]].energia){
					n_next = juego.triangulos[i].faros[0];
				}else{
					n_next = juego.triangulos[i].faros[2];
				}
			}else if (n_turn != d2){
				//d1 y d0 minimos
				if( juego.faros[juego.triangulos[i].faros[1]].energia <= juego.faros[juego.triangulos[i].faros[0]].energia){
					n_next = juego.triangulos[i].faros[1];
				}else{
					n_next = juego.triangulos[i].faros[0];
				}
			}else{
				//todos mínimo
				e0 = juego.faros[juego.triangulos[i].faros[0]].energia;
				e1 = juego.faros[juego.triangulos[i].faros[1]].energia;
				e2 = juego.faros[juego.triangulos[i].faros[2]].energia;
				if(e0 <= e1 && e0 <= e2){
					n_next = juego.triangulos[i].faros[0];
				}else if(e1 <= e2){
					n_next = juego.triangulos[i].faros[1];
				}else{
					n_next = juego.triangulos[i].faros[2];
				}
			}
			break;
		case 1:
			//controlo 1
			if(juego.faros[cnd[0]].llave == TRUE){
				//Tengo llave (0 el de llave)
				//df0 = juego.faros[cnd[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
				df1 = juego.faros[otr[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
				df2 = juego.faros[otr[1]].distancia[jugador.posicion[0]][jugador.posicion[1]];
				d01 = juego.faros[cnd[0]].distancia[juego.faros[otr[0]].posicion[0]][juego.faros[otr[0]].posicion[1]];
				d12 = juego.faros[otr[0]].distancia[juego.faros[otr[1]].posicion[0]][juego.faros[otr[1]].posicion[1]];
				d20 = juego.faros[otr[1]].distancia[juego.faros[cnd[0]].posicion[0]][juego.faros[cnd[0]].posicion[1]];
				d1 = fmin(df1+d01+d20+6,df1+d12+d20+5);
				d2 = fmin(df2+d20+d01+6,df2+d12+d01+5);
				n_connect = cnd[0];
				if(d1<d2){
					n_next = otr[0];
					n_turn = d1;
				}else if(d2<d1){
					n_next = otr[1];
					n_turn = d2;
				}else{
					//son iguales
					n_turn = d1;
					if(juego.faros[otr[0]].energia <= juego.faros[otr[1]].energia){
						n_next = otr[0];
					}else{
						n_next = otr[1];
					}
					
				}
			}else{
				//no tengo llave, como 0
				n_connect = juego.faros_count;
				df0 = juego.faros[cnd[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
				df1 = juego.faros[otr[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
				df2 = juego.faros[otr[1]].distancia[jugador.posicion[0]][jugador.posicion[1]];
				d01 = juego.faros[cnd[0]].distancia[juego.faros[otr[0]].posicion[0]][juego.faros[otr[0]].posicion[1]];
				d12 = juego.faros[otr[0]].distancia[juego.faros[otr[1]].posicion[0]][juego.faros[otr[1]].posicion[1]];
				d20 = juego.faros[otr[1]].distancia[juego.faros[cnd[0]].posicion[0]][juego.faros[cnd[0]].posicion[1]];

				//minimo de 0
				d0 = fmin((df0+juego.triangulos[i].perimetro+6), fmin((df0+(2*d01)+d20+7),(df0+(2*d20)+d01+7)));
				//minimo de 1
				d1 = fmin((df1+juego.triangulos[i].perimetro+6), fmin((df1+(2*d01)+d12+7), (df1+(2*d12)+d01+7)));
				//minimo de 2
				d2 = fmin((df2+juego.triangulos[i].perimetro+6), fmin((df2+(2*d20)+d12+7), (df2+(2*d12)+d20+7)));
				//minimo de todos
				n_turn = fmin(d0,fmin(d1,d2));
				if(n_turn!=d1 && n_turn!= d2){
					//hacia el 0
					n_next = juego.triangulos[i].faros[0];
				}else if(n_turn!= d0 && n_turn != d2){
					//hacia el 1
					n_next = juego.triangulos[i].faros[1];
				}else if(n_turn != d0 && n_turn != d1){
					//hacie el 2
					n_next = juego.triangulos[i].faros[2];
				}else if (n_turn != d0){
					//d1 y d2 minimos
					if( juego.faros[juego.triangulos[i].faros[1]].energia <= juego.faros[juego.triangulos[i].faros[2]].energia){
						n_next = juego.triangulos[i].faros[1];
					}else{
						n_next = juego.triangulos[i].faros[2];
					}
				}else if (n_turn != d1){
					//d0 y d2 minimos piorizar el que no tenmeos
					n_next = otr[1];
				}else if (n_turn != d2){
					//d1 y d0 minimos priorizar el que no tenemos
					n_next = otr[0];
				}else{
					//todos mínimo priorizar el que no tnemeos
					if(juego.faros[otr[0]].energia <= juego.faros[otr[1]].energia){
						n_next = otr[0];
					}else{
						n_next = otr[1];
					}
				}
			}
			break;
		case 2:
			//controlo 2
			if(is_conected(cnd[0],juego.faros[cnd[1]])==TRUE){
				//estan conectados
				if(juego.faros[cnd[0]].llave == TRUE && juego.faros[cnd[1]].llave == TRUE){
					//tenemos las dos llaves
					n_next = otr[0];
					n_turn = juego.faros[otr[0]].distancia[jugador.posicion[0]][jugador.posicion[1]]+3;
					//conectamos el que menos energía si tiene suficiente para 3 turnos
					if(juego.faros[cnd[0]].energia <= juego.faros[cnd[1]].energia){
						//cnd 0 menos energia
						if(juego.faros[cnd[0]].energia >= E_LOSS *3){
							n_connect = cnd[0];
						}else{
							n_connect = cnd[1];
						}
					}else{
						//cnd 1 menos energia
						if(juego.faros[cnd[1]].energia >= E_LOSS *3){
							n_connect = cnd[1];
						}else{
							n_connect = cnd[0];
						}
					}
				}else if(juego.faros[cnd[0]].llave == TRUE){
					//solo llave del 0
					df1 = juego.faros[cnd[1]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					df2 = juego.faros[otr[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					d12 = juego.faros[cnd[1]].distancia[juego.faros[otr[0]].posicion[0]][juego.faros[otr[0]].posicion[1]];
					d1 = df1 + d12 + 4;
					d2 = df2 + d12 + 4;
					if(d1<d2){
						//en caso de igualdad se prioriza el que no tenemos
						n_turn = d1;
						n_next = cnd[1];
						n_connect = juego.faros_count;
					}else{
						n_next = otr[0];
						n_turn = d2;
						n_connect = cnd[0];
					}

				}else if (juego.faros[cnd[1]].llave == TRUE){
					//solo llave del 1
					df0 = juego.faros[cnd[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					df2 = juego.faros[otr[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					d20 = juego.faros[otr[0]].distancia[juego.faros[cnd[0]].posicion[0]][juego.faros[cnd[0]].posicion[1]];
					d0 = df0 + d20 +4;
					d2 = df2 + d20 +4;
					if(d0<d2){
						//en caso de igualdad se prioriza el que no tenemos
						n_turn = d0;
						n_next = cnd[0];
						n_connect = juego.faros_count;
					}else{
						n_turn = d2;
						n_next = otr[0];
						n_connect = cnd[1];
					}
				}else{
					//ninguna llave como si 0
					n_connect = juego.faros_count;
					df0 = juego.faros[cnd[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					df1 = juego.faros[cnd[1]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					df2 = juego.faros[otr[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					d01 = juego.faros[cnd[0]].distancia[juego.faros[cnd[1]].posicion[0]][juego.faros[cnd[1]].posicion[1]];
					d12 = juego.faros[cnd[1]].distancia[juego.faros[otr[0]].posicion[0]][juego.faros[otr[0]].posicion[1]];
					d20 = juego.faros[otr[0]].distancia[juego.faros[cnd[0]].posicion[0]][juego.faros[cnd[0]].posicion[1]];
					d0 = fmin((df0+d01+d12+5),(df0+d20+d12+5));
					d1 = fmin((df1+d01+d20+5),(df1+d12+d20+5));
					d2 = fmin((df2+(2*d20)+d12+6),fmin((df2+d20+d01+d12+6),(df2+(2*d12)+d20+6)));
					n_turn = fmin(d0,fmin(d1,d2));
					if(n_turn!=d0 && n_turn!=d1){
						n_next = otr[0];
					}else if(n_turn!=d0 && n_turn!=d2){
						n_next = cnd[1];
					}else if(n_turn!=d1 && n_turn!=d2){
						n_next = cnd[0];
					}else if(n_turn!=d2){
						//empate 0 y 1
						//al de menor energi
						if(juego.faros[cnd[0]].energia<juego.faros[cnd[1]].energia){
							n_next=cnd[0];
						}else if(juego.faros[cnd[1]].energia<juego.faros[cnd[0]].energia){
							n_next=cnd[1];
						}else{
							//empate
							if(df0<df1){
								n_next=cnd[0];
							}else{
								n_next=cnd[1];
							}
						}
					}else{
						//empate con 2 o triple. Priorizar 2
						n_next = otr[0];
					}
				}
			}else{
				//no están conectados
				if(juego.faros[cnd[0]].llave == TRUE && juego.faros[cnd[1]].llave == TRUE){
					//ambas llaves
					df0 = juego.faros[cnd[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					df1 = juego.faros[cnd[1]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					df2 = juego.faros[otr[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					d0 = fmin((df0+d20+d12+6),(df0+d01+d12+6));
					d1 = fmin((df1+d01+d20+6),(df1+d12+d20+6));
					d2 = fmin((df2+d20+d01+6),(df2+d12+d01+6));
					//snprintf(msg,MAX_CANON,"2 no conectados y ambas llaves d0: %u d1: %u d2 %u", d0,d1,d2);
					//snd_log(msg);
					n_turn = fmin(d0,fmin(d1,d2));
					//snprintf(msg,MAX_CANON,"n_turn: %u", n_turn);
					//snd_log(msg);
					if(n_turn!=d0 && n_turn!=d1){
						//al 2
						n_next = otr[0];
						//conectar al de más enegia primero
						if(juego.faros[cnd[0]].energia<=juego.faros[cnd[1]].energia){
							n_connect = cnd[1];
						}else{
							n_connect = cnd[0];
						}
					}else if(n_turn!=d2 && n_turn != d1){
						//al 0
						n_next = cnd[0];
						n_connect = cnd[1];
					}else if(n_turn!=d2 && n_turn != d0){
						//al 1
						n_next = cnd[1];
						n_connect = cnd[0];
					}else if(n_turn!=d2){
						//empate 1 y 0 al menor energia
						if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
							n_next = cnd[0];
							n_connect = cnd[1];
						}else if(juego.faros[cnd[1]].energia < juego.faros[cnd[1]].energia){
							n_next = cnd[1];
							n_connect = cnd[0];
						}else{
							//misma energía, al más cercano
							if(df0<=df1){
								n_next = cnd[0];
								n_connect = cnd[1];
							}else{
								n_next = cnd[1];
								n_connect = cnd[0];
							}
						}
					}else{
						//empate con 2 o triples. Priorizar el que no tenemos
						n_next = otr[0];
						//conectar al de más enegia primero
			 			if(juego.faros[cnd[0]].energia<= juego.faros[cnd[1]].energia){
							n_connect = cnd[1];
						}else{
							n_connect = cnd[0];
						}
						
					}
				}else if(juego.faros[cnd[0]].llave == TRUE){
					//solo llave del 0
					n_connect = cnd[0];
					df1 = juego.faros[cnd[1]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					df2 = juego.faros[otr[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					d12 = juego.faros[cnd[1]].distancia[juego.faros[otr[0]].posicion[0]][juego.faros[otr[0]].posicion[1]];
					d01 = juego.faros[cnd[0]].distancia[juego.faros[cnd[1]].posicion[0]][juego.faros[cnd[1]].posicion[1]];
					d20 = juego.faros[otr[0]].distancia[juego.faros[cnd[0]].posicion[0]][juego.faros[cnd[0]].posicion[1]];
					d1 = fmin((df1+d01+d20+6),(df1+d12+d20+6));
					d2 = fmin((df2+d20+d01+6),(df2+d12+d01+6));
					if(d1<d2){
						n_turn = d1;
						n_next = cnd[1];
					}else{
						//en caso de empate priorizar el libre
						n_next = otr[0];
						n_turn = d2;
					}
				}else if(juego.faros[cnd[1]].llave == TRUE){
					//solo llave del 1
					n_connect = cnd[1];
					df0 = juego.faros[cnd[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					df2 = juego.faros[otr[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					d01 = juego.faros[cnd[0]].distancia[juego.faros[cnd[1]].posicion[0]][juego.faros[cnd[1]].posicion[1]];
					d12 = juego.faros[cnd[1]].distancia[juego.faros[otr[0]].posicion[0]][juego.faros[otr[0]].posicion[1]];
					d20 = juego.faros[otr[0]].distancia[juego.faros[cnd[0]].posicion[0]][juego.faros[cnd[0]].posicion[1]];
					d0 = fmin((df0+d01+d12+6),(df0+d20+d12+6));
					d2 = fmin((df2+d20+d01+6),(df2+d12+d01+6));
					if(d0<d2){
						n_turn = d0;
						n_next = cnd[0];
					}else{
						n_turn = d2;
						n_next = otr[0];
					}
				}else{
					//ninguna llave, como si 0
					n_connect = juego.faros_count;
					df0 = juego.faros[cnd[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					df1 = juego.faros[cnd[1]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					df2 = juego.faros[otr[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					d01 = juego.faros[cnd[0]].distancia[juego.faros[cnd[1]].posicion[0]][juego.faros[cnd[1]].posicion[1]];
					d12 = juego.faros[cnd[1]].distancia[juego.faros[otr[0]].posicion[0]][juego.faros[otr[0]].posicion[1]];
					d20 = juego.faros[otr[0]].distancia[juego.faros[cnd[0]].posicion[0]][juego.faros[cnd[0]].posicion[1]];
					d0 = fmin((df0+d01+d12+5),(df0+d20+d12+5));
					d1 = fmin((df1+d01+d20+5),(df1+d12+d20+5));
					d2 = fmin((df2+(2*d20)+d12+6),fmin((df2+d20+d01+d12+6),(df2+(2*d12)+d20+6)));
					n_turn = fmin(d0,fmin(d1,d2));
					if(n_turn!=d0 && n_turn!=d1){
						n_next = otr[0];
					}else if(n_turn!=d0 && n_turn!=d2){
						n_next = cnd[1];
					}else if(n_turn!=d1 && n_turn!=d2){
						n_next = cnd[0];
					}else if(n_turn!=d2){
						//empate 0 y 1
						//al de menor energia
						if(juego.faros[cnd[0]].energia<juego.faros[cnd[1]].energia){
							n_next=cnd[0];
						}else if(juego.faros[cnd[1]].energia<juego.faros[cnd[0]].energia){
							n_next=cnd[1];
						}else{
							//empate
							if(df0<df1){
								n_next=cnd[0];
							}else{
								n_next=cnd[1];
							}
						}
					}else{
						//empate con 2 o triple. Priorizar 2
						n_next = otr[0];
					}
				}	
				
			}
			break;
		case 3:
			//controlo 3
			if(is_conected(cnd[0],juego.faros[cnd[1]])==TRUE && is_conected(cnd[1],juego.faros[cnd[2]])==TRUE && is_conected(cnd[2],juego.faros[cnd[0]])==TRUE){
				//ya está el triángulo
				n_turn = juego.maxx*juego.maxy;
			}else{
				//falta alguna conexión
				if(is_conected(cnd[1],juego.faros[cnd[2]])==TRUE && is_conected(cnd[2],juego.faros[cnd[0]])==TRUE){
					//falta la 0-1
					if(juego.faros[cnd[0]].llave == TRUE && juego.faros[cnd[1]].llave == TRUE){
						//tenemos la llave de los dos
						d0 = juego.faros[cnd[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
						d1 = juego.faros[cnd[1]].distancia[jugador.posicion[0]][jugador.posicion[1]];
						if(d0<d1){
							n_next = cnd[0];
							n_turn = d0+2;
							n_connect = cnd[1];
						}else if(d1<d0){
							n_next = cnd[1];
							n_turn = d1+2;
							n_connect = cnd[0];
						}else{
							//si son iguales al de menor energia
							if(juego.faros[cnd[0]].energia<juego.faros[cnd[1]].energia){
								n_next = cnd[0];
								n_turn = d0+2;
								n_connect = cnd[1];
							}else{
								n_next = cnd[1];
								n_turn = d1+2;
								n_connect = cnd[0];
							}
						}
					}else if(juego.faros[cnd[0]].llave == TRUE){
						//solo llave del 0
						n_turn = juego.faros[cnd[1]].distancia[jugador.posicion[0]][jugador.posicion[1]]+2;
						n_next = cnd[1];
						n_connect = cnd[0];
					}else if(juego.faros[cnd[1]].llave == TRUE){
						//solo llave del 1
						n_turn = juego.faros[cnd[0]].distancia[jugador.posicion[0]][jugador.posicion[1]]+2;
						n_next = cnd[0];
						n_connect = cnd[1];
					}else{
						//ninguna llave
						n_connect = juego.faros_count;
						d0 = juego.faros[cnd[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
						d1 = juego.faros[cnd[1]].distancia[jugador.posicion[0]][jugador.posicion[1]];
						d01 = juego.faros[cnd[0]].distancia[juego.faros[cnd[1]].posicion[0]][juego.faros[cnd[1]].posicion[1]];
						if(d0<d1){
							n_next = cnd[0];
							n_turn = d0+d01+3;
						}else if(d1<d0){
							n_next = cnd[1];
							n_turn = d1+d01+3;
						}else{
							//empate atacar al de menor energia
							if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
								n_next = cnd[0];
								n_turn = d0+d01+3;
							}else{
								n_next = cnd[1];
								n_turn = d1+d01+3;
							}
						}
					}
				}else if(is_conected(cnd[0],juego.faros[cnd[1]])==TRUE && is_conected(cnd[0],juego.faros[cnd[2]])==TRUE){
					//falta la 1-2
					if(juego.faros[cnd[1]].llave == TRUE && juego.faros[cnd[2]].llave == TRUE){
						//tenemos ambas llaves
						d1 = juego.faros[cnd[1]].distancia[jugador.posicion[0]][jugador.posicion[1]];
						d2 = juego.faros[cnd[2]].distancia[jugador.posicion[0]][jugador.posicion[0]];
						if(d1<d2){
							n_turn = d1+2;
							n_next = cnd[1];
							n_connect = cnd[2];
						}else if(d2<d1){
							n_turn = d2+2;
							n_next = cnd[2];
							n_connect = cnd[1];
						}else{
							//en caso de empate al de menor energia
							if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
								n_next = cnd[1];
								n_turn = d1+2;
								n_connect = cnd[2];
							}else{
								n_next = cnd[2];
								n_turn = d2+2;
								n_connect = cnd[1];
							}
						}
					}else if(juego.faros[cnd[1]].llave == TRUE){
						//solo llave del 1
						n_next = cnd[2];
						n_turn = juego.faros[cnd[2]].distancia[jugador.posicion[0]][jugador.posicion[1]]+2;
						n_connect = cnd[1];
					}else if(juego.faros[cnd[2]].llave == TRUE){
						//solo llave del 2
						n_next = cnd[1];
						n_turn = juego.faros[cnd[1]].distancia[jugador.posicion[0]][jugador.posicion[1]]+2;
						n_connect = cnd[2];
					}else{
						//ninguna llave
						n_connect = juego.faros_count;
						d1 = juego.faros[cnd[1]].distancia[jugador.posicion[0]][jugador.posicion[1]];
						d2 = juego.faros[cnd[2]].distancia[jugador.posicion[0]][jugador.posicion[1]];
						d12 = juego.faros[cnd[1]].distancia[juego.faros[cnd[2]].posicion[0]][juego.faros[cnd[2]].posicion[1]];
						if(d1<d2){
							n_next = cnd[1];
							n_turn = d1+d12+3;
						}else if(d2<d1){
							n_next = cnd[2];
							n_turn = d2+d12+3;
						}else{
							//empate atacar al de menor energia
							if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
								n_next = cnd[1];
								n_turn = d1+d12+3;
							}else{
								n_next = cnd[2];
								n_turn = d2+d12+3;
							}
						}
					}
				}else if(is_conected(cnd[0],juego.faros[cnd[1]])==TRUE && is_conected(cnd[2],juego.faros[cnd[1]])==TRUE){
					//falta la 2-0
					if(juego.faros[cnd[0]].llave ==TRUE && juego.faros[cnd[2]].llave == TRUE){
						//ambas llaves
						d0 = juego.faros[cnd[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
						d2 = juego.faros[cnd[2]].distancia[jugador.posicion[0]][jugador.posicion[1]];
						if(d0<d2){
							n_next = cnd[0];
							n_connect = cnd[2];
							n_turn = d0+2;
						}else if(d2<d0){
							n_next = cnd[2];
							n_connect=cnd[0];
							n_turn = d2+2;
						}else{
							//misma distancia ir al de menor energia
							if(juego.faros[cnd[0]].energia<juego.faros[cnd[2]].energia){
								n_next = cnd[0];
								n_turn = d0+2;
								n_connect = cnd[2];
							}else{
								n_next = cnd[2];
								n_turn = d2+2;
								n_connect = cnd[0];
							}
						}
					}else if(juego.faros[cnd[0]].llave == TRUE){
						//solo llave del 0
						n_connect = cnd[0];
						n_next = cnd[2];
						n_turn = juego.faros[cnd[2]].distancia[jugador.posicion[0]][jugador.posicion[1]]+2;
					}else if(juego.faros[cnd[2]].llave == TRUE){
						//solo llave del 2
						n_connect = cnd[2];
						n_next = cnd[0];
						n_turn = juego.faros[cnd[0]].distancia[jugador.posicion[0]][jugador.posicion[1]]+2;
					}else{
						//ninguna llave
						n_connect = juego.faros_count;
						d0 = juego.faros[cnd[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
						d2 = juego.faros[cnd[2]].distancia[jugador.posicion[0]][jugador.posicion[1]];
						d20 = juego.faros[cnd[0]].distancia[juego.faros[cnd[2]].posicion[0]][juego.faros[cnd[2]].posicion[1]];

						if(d0<d2){
							n_next = cnd[0];
							n_turn = d0+d20+3;
						}else if(d2<d0){
							n_next = cnd[2];
							n_turn = d2+d20+3;
						}else{
							//iguales al de menor energia
							if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
								n_next = cnd[0];
								n_turn = d0+d20+3;
							}else{
								n_next = cnd[2];
								n_turn = d2+d20+3;
							}
						}
					}
				}else if(is_conected(cnd[0],juego.faros[cnd[1]])==TRUE){
					//falta 1-2 y 2-0
					df0 = juego.faros[cnd[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					df1 = juego.faros[cnd[1]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					df2 = juego.faros[cnd[2]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					d01 = juego.faros[cnd[0]].distancia[juego.faros[cnd[1]].posicion[0]][juego.faros[cnd[1]].posicion[1]];
					d12 = juego.faros[cnd[1]].distancia[juego.faros[cnd[2]].posicion[0]][juego.faros[cnd[2]].posicion[1]];
					d20 = juego.faros[cnd[2]].distancia[juego.faros[cnd[0]].posicion[0]][juego.faros[cnd[0]].posicion[1]];
					if(juego.faros[cnd[0]].llave == TRUE && juego.faros[cnd[1]].llave == TRUE && juego.faros[cnd[2]].llave == TRUE){
						//las 3 llaves
						d2 = df2+3;
						d1 = fmin((df1+d12+4),(df1+d01+4));
						d0 = fmin((df0+d20+4),(df0+d01+4));
						n_turn = fmin(d2,fmin(d1,d0));
						if(n_turn!= d2 && n_turn != d1){
							//al 0
							n_next = cnd[0];
							n_connect = cnd[2];
						}else if(n_turn != d1 && n_turn != d0){
							//al 2
							n_next = cnd[2];
							//conectar al de más energia
							if(juego.faros[cnd[0]].energia<juego.faros[cnd[1]].energia){
								n_connect = cnd[1];
							}else{
								n_connect = cnd[0];
							}
						}else if(n_turn != d0 && n_turn != d2){
							//al 1
							n_next = cnd[1];
							n_connect = cnd[2];
						}else if(n_turn != d2){
							// 0 y 1 iguales al menor df
							n_connect = cnd[2];
							if(df0<df1){
								n_next = cnd[0];
							}else if(df1<df0){
								n_next = cnd[1];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next=cnd[0];
								}else{
									n_next = cnd[1];
								}
							}
						}else if(n_turn != d1){
							// 0 y 2 iguales al menor df
							if(df0<df2){
								n_next = cnd[0];
								n_connect = cnd[2];
							}else if(df2<df0){
								n_next = cnd[2];
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_connect = cnd[1];
								}else{
									n_connect = cnd[0];
								}
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_connect = cnd[2];
									n_next = cnd[0];
								}else{
									n_next = cnd[2];
									if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
										n_connect = cnd[1];
									}else{
										n_connect = cnd[0];
									}
								}
							}
						}else{
							// 2 y 1 iguales al menor df
							if(df1<df2){
								n_next = cnd[1];
								n_connect = cnd[2];
							}else if(df2<df1){
								n_next = cnd[2];
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_connect = cnd[1];
								}else{
									n_connect = cnd[0];
								}
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_connect = cnd[2];
									n_next = cnd[1];
								}else{
									n_next = cnd[2];
									if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
										n_connect = cnd[1];
									}else{
										n_connect = cnd[0];
									}
								}
							}
						}
					}else if(juego.faros[cnd[0]].llave == TRUE && juego.faros[cnd[1]].llave == TRUE){
						//llaves del 0 y 1
						n_turn = df2+3;
						n_next = cnd[2];
						if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
							n_connect = cnd[1];
						}else{
							n_connect = cnd[0];
						}
					}else if(juego.faros[cnd[1]].llave == TRUE && juego.faros[cnd[2]].llave == TRUE){
						//llaves del 1 y 2
						d0 = df0+d20+4;
						d1 = fmin((df1+d12+d20+4),(df1+d01+d20+4));
						d2 = df2+d20+4;
						n_turn = fmin(d0,fmin(d1,d2));
						if(n_turn != d0 && n_turn != d1){
							//al 2
							n_next = cnd[2];
							n_connect = cnd[1];
						}else if(n_turn != d1 && n_turn != d2) {
							//al 0
							n_next = cnd[0];
							n_connect = cnd[2];
						}else if(n_turn != d2 && n_turn != d0){
							//al 1
							n_next = cnd[1];
							n_connect = cnd[2];
						}else if(n_turn != d0){
							//1 y 2 iguales al df más bajo
							if(df2 < df1){
								n_next = cnd[2];
								n_connect = cnd[1];
							}else if(df1 < df2){
								n_next = cnd[1];
								n_connect = cnd[2];
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									n_connect = cnd[2];
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}
						}else if(n_turn != d1){
							//0 y 2 iguales al df más bajo
							if(df0 < df2){
								n_next = cnd[0];
								n_connect = cnd[2];
							}else if(df2 < df0){
								n_next = cnd[2];
								n_connect = cnd[1];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[2];
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}
						}else if(n_turn != d2){
							//0 y 1 iguales al df más bajo
							n_connect = cnd[2];
							if(df0 < df1){
								n_next = cnd[0];
							}else if(df1 < df0){
								n_next = cnd[1];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[1];
								}
							}
						}else{
							//triple empate al df más bajo
							if(df0 < df1 && df0 < df2){
								n_next = cnd[0];
								n_connect = cnd[2];
							}else if(df1 < df0 && df1 < df2){
								n_next = cnd[1];
								n_connect = cnd[2];
							}else if(df2 < df0 && df2 < df1){
								n_next = cnd[2];
								n_connect = cnd[1];
							}else if(df0>df1 && df0>df2){
								//cualquier empate priorizar por energia
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									n_connect = cnd[2];
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}else if(df1>df0 && df1>df2){
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[2];
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}else if(df2>df0 && df2>df1){
								n_connect = cnd[2];
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[1];
								}
							}else{
								//triple empate
								if(juego.faros[cnd[0]].energia<juego.faros[cnd[1]].energia && juego.faros[cnd[0]].energia< juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[2];
								}else if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									n_connect = cnd[2];
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}
						}
					}else if(juego.faros[cnd[2]].llave == TRUE && juego.faros[cnd[0]].llave == TRUE){
						//llaves 2 y 0
						d0 = fmin((df0+d20+d12+4),(df0+d01+d12+4));
						d1 = df1+d12+4;
						d2 = df2+d12+4;
						n_turn = fmin(d0,fmin(d1,d2));
						if(n_turn != d0 && n_turn != d1){
							//al 2
							n_next = cnd[2];
							n_connect = cnd[0];
						}else if(n_turn != d1 && n_turn != d2){
							//al 0
							n_next = cnd[0];
							n_connect = cnd[2];
						}else if(n_turn != d2 && n_turn != d0){
							//al 1
							n_next = cnd[1];
							n_connect = cnd[2];
						}else if(n_turn != d0){
							// 1 y 2 iguales al de menos df
							if(df1<df2){
								n_next = cnd[1];
								n_connect = cnd[2];
							}else if(df2<df1){
								n_next = cnd[2];
								n_connect = cnd[0];
							}else{
								if(juego.faros[cnd[1]].energia<juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									n_connect = cnd[2];
								}else{
									n_next = cnd[2];
									n_connect = cnd[0];
								}
							}
						}else if(n_turn != d1){
							// 0 y 2 iguales
							if(df0<df2){
								n_next = cnd[0];
								n_connect = cnd[2];
							}else if(df2<df0){
								n_next = cnd[2];
								n_connect = cnd[0];
							}else{
								if(juego.faros[cnd[0]].energia<juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[2];
								}else{
									n_next = cnd[2];
									n_connect = cnd[0];
								}
							}
						}else if(n_turn != d2){
							// 0 y 1 iguales
							if(df0<df1){
								n_next = cnd[0];
								n_connect = cnd[2];
							}else if(df2<df1){
								n_next = cnd[1];
								n_connect = cnd[2];
							}else{
								if(juego.faros[cnd[1]].energia<juego.faros[cnd[0]].energia){
									n_next = cnd[1];
									n_connect = cnd[2];
								}else{
									n_next = cnd[0];
									n_connect = cnd[2];
								}
							}
						}else{
							//triple empate al de menor df
							if(df0<df1 && df0<df2){
								n_next = cnd[0];
								n_connect = cnd[2];
							}else if(df1<df0 && df1<df2){
								n_next = cnd[1];
								n_connect = cnd[2];
							}else if(df2<df0 && df2<df1){
								n_next = cnd[2];
								n_connect = cnd[0];
							}else if(df0>df1 && df0>df2){
								if(juego.faros[cnd[1]].energia<juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									n_connect = cnd[2];
								}else{
									n_next = cnd[2];
									n_connect = cnd[0];
								}
							}else if(df1>df0 && df1>df2){
								if(juego.faros[cnd[0]].energia<juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[2];
								}else{
									n_next = cnd[2];
									n_connect = cnd[0];
								}
							}else if(df2>df0 && df2>df1){
								if(juego.faros[cnd[1]].energia<juego.faros[cnd[0]].energia){
									n_next = cnd[1];
									n_connect = cnd[2];
								}else{
									n_next = cnd[0];
									n_connect = cnd[2];
								}
							}else{
								//cualquier empate por energia
								if(juego.faros[cnd[0]].energia<juego.faros[cnd[1]].energia && juego.faros[cnd[0]].energia<juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[2];
								}else if(juego.faros[cnd[1]].energia<juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									n_connect = cnd[2];
								}else{
									n_next = cnd[2];
									n_connect = cnd[0];
								}
							}
						}
					}else if(juego.faros[cnd[0]].llave == TRUE){
						//solo llave del 0
						if(df1<df2){
							n_turn = df1+d12+4;
							n_next = cnd[1];
							n_connect = juego.faros_count;
						}else{
							//2 o empate priorizamos conexión rápida
							n_turn = df2+d12+4;
							n_next = cnd[2];
							n_connect = cnd[0];
						}
					}else if(juego.faros[cnd[1]].llave == TRUE){
						//solo llave del 1
						if(df0<df2){
							n_turn = df0+d20+4;
							n_next = cnd[0];
							n_connect = juego.faros_count;
						}else{
							//2 o empate priorizamos conexión rápida
							n_turn = df2+d20+4;
							n_next = cnd[2];
							n_connect = cnd[1];
						}
					}else if(juego.faros[cnd[2]].llave == TRUE){
						//solo llave del 2
						d0 = fmin((df0+d01+d12+4),(df0+d20+d12+4));
						d1 = fmin((df1+d01+d12+4),(df1+d12+d20+4));
						n_turn = fmin(d0,d1);
						n_connect = cnd[2];
						if(n_turn != d1){
							n_next = cnd[0];
						}else if(n_turn != d0){
							n_next = cnd[1];
						}else{
							if(df0<df1){
								n_next = cnd[0];
							}else if(df1<df0){
								n_next = cnd[1];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[1];
								}
							}
						}
						
					}else{
						//ninguna llave como si no hubiera nada
						n_connect = juego.faros_count;
						d0 = fmin((df0+d20+d12+4),(df0+d01+d12+3));
						d1 = fmin((df1+d12+d20+4),(df1+d01+d20+3));
						d2 = fmin((df2+d20+d20+d12+6),fmin((df2+d20+d01+d12+6),fmin((df2+d12+d01+d20+6),(df2+d12+d12+d20+6))));
						n_turn = fmin(d0,fmin(d1,d2));
						if(n_turn != d0 && n_turn != d1){
							//al 2
							n_next = cnd[2];
						}else if(n_turn != d1 && n_turn != d2){
							//al 0
							n_next = cnd[0];
						}else if(n_turn =! d2 && n_turn != d0){
							//al 1
							n_next = cnd[1];
						}else if(n_turn != d0){
							// iguales 1 y 2
							if(df1<df2){
								n_next = cnd[1];
							}else if(df2<df1){
								n_next = cnd[2];
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
								}else{
									n_next = cnd[2];
								}
							}
						}else if(n_turn != d1){
							//iguales 0 y 2
							if(df0<df2){
								n_next = cnd[0];
							}else if(df2<df0){
								n_next = cnd[2];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[2];
								}
							}
						}else if(n_turn != d2){
							//iguales 0 y 1
							if(df0<df1){
								n_next = cnd[0];
							}else if(df1<df0){
								n_next = cnd[1];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[1];
								}
							}
						}else{
							//triple empate
							if(df0<df1 && df0<df2){
								n_next = cnd[0];
							}else if(df1<df0 && df1<df2){
								n_next = cnd[1];
							}else if(df2<df0 && df2<df1){
								n_next = cnd[2];
							}else if(df0>df1 && df0>df2){
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
								}else{
									n_next = cnd[2];
								}
							}else if(df1>df0 && df1>df2){
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[2];
								}
							}else if(df2>df0 && df2>df1){
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[1];
								}
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia && juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
								}else if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
								}else{
									n_next = cnd[2];
								}
							}
						}
					}
				}else if(is_conected(cnd[1],juego.faros[cnd[2]])==TRUE){
					//falta 0-1 y 2-0
					df0 = juego.faros[cnd[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					df1 = juego.faros[cnd[1]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					df2 = juego.faros[cnd[2]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					d01 = juego.faros[cnd[0]].distancia[juego.faros[cnd[1]].posicion[0]][juego.faros[cnd[1]].posicion[1]];
					d12 = juego.faros[cnd[1]].distancia[juego.faros[cnd[2]].posicion[0]][juego.faros[cnd[2]].posicion[1]];
					d20 = juego.faros[cnd[2]].distancia[juego.faros[cnd[0]].posicion[0]][juego.faros[cnd[0]].posicion[1]];
					if(juego.faros[cnd[0]].llave == TRUE && juego.faros[cnd[1]].llave == TRUE && juego.faros[cnd[2]].llave == TRUE){
						//las 3 llaves
						d2 = fmin((df2+d12+4),(df2+d20+4));
						d1 = fmin((df1+d12+4),(df1+d01+4));
						d0 = df0+3;
						n_turn = fmin(d2,fmin(d1,d0));
						if(n_turn!= d2 && n_turn != d1){
							//al 0
							n_next = cnd[0];
							//conectar al de más energia
							if(juego.faros[cnd[2]].energia<juego.faros[cnd[1]].energia){
								n_connect = cnd[1];
							}else{
								n_connect = cnd[2];
							}
						}else if(n_turn != d1 && n_turn != d0){
							//al 2
							n_next = cnd[2];
							n_connect = cnd[0];
						}else if(n_turn != d0 && n_turn != d2){
							//al 1
							n_next = cnd[1];
							n_connect = cnd[0];
						}else if(n_turn != d2){
							// 0 y 1 iguales al de menor df
							if(df0<df1){
								n_next = cnd[0];
								//conectar al de más energia
								if(juego.faros[cnd[2]].energia<juego.faros[cnd[1]].energia){
									n_connect = cnd[1];
								}else{
									n_connect = cnd[2];
								}
							}else if(df1<df0){
								n_next = cnd[1];
								n_connect = cnd[0];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next=cnd[0];
									//conectar al de más energia
									if(juego.faros[cnd[2]].energia<juego.faros[cnd[1]].energia){
										n_connect = cnd[1];
									}else{
										n_connect = cnd[2];
									}
								}else{
									n_next = cnd[1];
									n_connect = cnd[0];
								}
							}
						}else if(n_turn != d1){
							// 0 y 2 iguales al de menor df
							if(df0<df2){
								n_next = cnd[0];
								//conectar al de más energia
								if(juego.faros[cnd[2]].energia<juego.faros[cnd[1]].energia){
									n_connect = cnd[1];
								}else{
									n_connect = cnd[2];
								}
							}else if(df2<df0){
								n_next = cnd[2];
								n_connect = cnd[0];
							}else{
								if(juego.faros[cnd[2]].energia < juego.faros[cnd[0]].energia){
									n_connect = cnd[0];
									n_next = cnd[2];
								}else{
									n_next = cnd[0];
									//conectar al de más energia
									if(juego.faros[cnd[2]].energia<juego.faros[cnd[1]].energia){
										n_connect = cnd[1];
									}else{
										n_connect = cnd[2];
									}
								}
							}
						}else if(n_turn != d0){
							// 2 y 1 iguales al de menor df
							n_connect = cnd[0];
							if(df1<df2){
								n_next = cnd[1];
							}else if(df2<df1){
								n_next = cnd[2];
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
								}else{
									n_next = cnd[2];
								}
							}
						}else{
							//triple empate
							if(df0<df1 && df0<df2){
								n_next = cnd[0];
								if(juego.faros[cnd[2]].energia < juego.faros[cnd[1]].energia){
									n_connect = cnd[1];
								}else{
									n_connect = cnd[2];
								}
							}else if(df1<df0 && df1<df2){
								n_next = cnd[1];
								n_connect = cnd[0];
							}else if(df2<df0 && df2<df1){
								n_next = cnd[2];
								n_connect = cnd[0];
							}else if(df0>df1 && df0>df2){
								n_connect = cnd[0];
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next =cnd[1];
								}else{
									n_next =cnd[2];
								}
							}else if(df1>df0 && df1>df2){
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									if(juego.faros[cnd[2]].energia < juego.faros[cnd[1]].energia){
										n_connect = cnd[1];
									}else{
										n_connect = cnd[2];
									}
								}else{
									n_next = cnd[2];
									n_connect = cnd[0];
								}
							}else if(df2>df0 && df2>df1){
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
									if(juego.faros[cnd[2]].energia < juego.faros[cnd[1]].energia){
										n_connect = cnd[1];
									}else{
										n_connect = cnd[2];
									}
								}else{
									n_next = cnd[1];
									n_connect = cnd[0];
								}
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia && juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									if(juego.faros[cnd[2]].energia < juego.faros[cnd[1]].energia){
										n_connect = cnd[1];
									}else{
										n_connect = cnd[2];
									}
								}else if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									n_connect = cnd[0];
								}else{
									n_next = cnd[2];
									n_connect = cnd[0];
								}
							}
						}
					}else if(juego.faros[cnd[0]].llave == TRUE && juego.faros[cnd[1]].llave == TRUE){
						//llaves del 0 y 1
						d0 = df0+d20+4;
						d1 = fmin((df1+d12+d20+4),(df1+d01+d20+4));
						d2 = df2+d20+4;
						n_turn = fmin(d0,fmin(d1,d2));
						if(n_turn != d0 && n_turn != d1){
							//al 2
							n_next = cnd[2];
							n_connect = cnd[0];
						}else if(n_turn != d1 && n_turn != d2){
							//al 0
							n_next = cnd[0];
							n_connect = cnd[1];
						}else if(n_turn != d2 && n_turn != d0){
							//al 1
							n_next = cnd[1];
							n_connect = cnd[0];
						}else if(n_turn != d0){
							//empate 1 y 2
							n_connect = cnd[0];
							if(df1<df2){
								n_next = cnd[1];
							}else if(df2<df1){
								n_next = cnd[2];
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
								}else{
									n_next = cnd[2];
								}
							}
						}else if(n_turn != d1){
							//empate 0 y 2
							if(df0<df2){
								n_next = cnd[0];
								n_connect = cnd[1];
							}else if(df2<df0){
								n_next = cnd[2];
								n_connect = cnd[0];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[1];
								}else{
									n_next = cnd[2];
									n_connect = cnd[0];
								}
							}
						}else if(n_turn != d2){
							//empate 0 y 1
							if(df0<df1){
								n_next = cnd[0];
								n_connect = cnd[1];
							}else if(df1<df0){
								n_next = cnd[1];
								n_connect = cnd[0];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
									n_connect = cnd[1];
								}else{
									n_next = cnd[1];
									n_connect = cnd[0];
								}
							}

						}else{
							//triple empate
							if(df0<df1 && df0<df2){
								n_next = cnd[0];
								n_connect = cnd[1];
							}else if(df1<df0 && df1<df2){
								n_next = cnd[1];
								n_connect = cnd[0];
							}else if(df2<df0 && df2<df1){
								n_next = cnd[2];
								n_connect = cnd[0];
							}else if(df0>df1 && df0>df2){
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									n_connect = cnd[0];
								}else{
									n_next = cnd[2];
									n_connect = cnd[0];
								}
							}else if(df1>df0 && df1>df2){
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[1];
								}else{
									n_next = cnd[2];
									n_connect = cnd[0];
								}
							}else if(df2>df0 && df2>df1){
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
									n_connect = cnd[1];
								}else{
									n_next = cnd[1];
									n_connect = cnd[0];
								}
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia && juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[1];
								}else if(juego.faros[cnd[1]].energia <juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									n_connect = cnd[0];
								}else{
									n_next = cnd[2];
									n_connect = cnd[0];
								}
							}
						}
					}else if(juego.faros[cnd[1]].llave == TRUE && juego.faros[cnd[2]].llave == TRUE){
						//llaves del 1 y 2
						n_turn = df0+3;
						n_next = cnd[0];
						if(juego.faros[cnd[2]].energia < juego.faros[cnd[1]].energia){
							n_connect = cnd[1];
						}else{
							n_connect = cnd[2];
						}
					}else if(juego.faros[cnd[2]].llave == TRUE && juego.faros[cnd[0]].llave == TRUE){
						//llaves 2 y 0
						d0 = df0+d01+4;
						d1 = df1+d01+4;
						d2 = fmin((df2+d12+d01+4),(df2+d20+d01+4));
						n_turn = fmin(d0,fmin(d1,d2));
						if(n_turn != d0 && n_turn != d1){
							//al 2
							n_next = cnd[2];
							n_connect = cnd[0];
						}else if(n_turn != d1 && n_turn != d2){
							//al 0
							n_next = cnd[0];
							n_connect = cnd[2];
						}else if(n_turn != d2 && n_turn != d0){
							//al 1
							n_next = cnd[1];
							n_connect = cnd[0];
						}else if(n_turn != d0){
							// 1 y 2 iguales al de menos df
							n_connect = cnd[0];
							if(df1<df2){
								n_next = cnd[1];
							}else if(df2<df1){
								n_next = cnd[2];
							}else{
								if(juego.faros[cnd[1]].energia<juego.faros[cnd[2]].energia){
									n_next = cnd[1];
								}else{
									n_next = cnd[2];
								}
							}
						}else if(n_turn != d1){
							// 0 y 2 iguales
							if(df0<df2){
								n_next = cnd[0];
								n_connect = cnd[2];
							}else if(df2<df0){
								n_next = cnd[2];
								n_connect = cnd[0];
							}else{
								if(juego.faros[cnd[0]].energia<juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[2];
								}else{
									n_next = cnd[2];
									n_connect = cnd[0];
								}
							}
						}else if(n_turn != d2){
							// 0 y 1 iguales
							if(df0<df1){
								n_next = cnd[0];
								n_connect = cnd[2];
							}else if(df2<df1){
								n_next = cnd[1];
								n_connect = cnd[0];
							}else{
								if(juego.faros[cnd[1]].energia<juego.faros[cnd[0]].energia){
									n_next = cnd[1];
									n_connect = cnd[0];
								}else{
									n_next = cnd[0];
									n_connect = cnd[2];
								}
							}
						}else{
							//triple empate al de menor df
							if(df0<df1 && df0<df2){
								n_next = cnd[0];
								n_connect = cnd[2];
							}else if(df1<df0 && df1<df2){
								n_next = cnd[1];
								n_connect = cnd[0];
							}else if(df2<df0 && df2<df1){
								n_next = cnd[2];
								n_connect = cnd[0];
							}else{
								//cualquier empate por energia
								if(juego.faros[cnd[0]].energia<juego.faros[cnd[1]].energia && juego.faros[cnd[0]].energia<juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[2];
								}else if(juego.faros[cnd[1]].energia<juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									n_connect = cnd[0];
								}else{
									n_next = cnd[2];
									n_connect = cnd[0];
								}
							}
						}
					}else if(juego.faros[cnd[0]].llave == TRUE){
						//solo llave del 0
						n_connect = cnd[0];
						d1 = fmin((df1+d01+d20+4),(df1+d12+d20+4));
						d2 = fmin((df2+d20+d01+4),(df2+d12+d01+4));
						if(d1<d2){
							n_turn = d1;
							n_next = cnd[1];
						}else if(d2<d1){
							n_turn = d2;
							n_next = cnd[2];
						}else{
							//empate
							n_turn = d1;
							if(df1<df2){
								n_next = cnd[1];
							}else if(df2<df1){
								n_next = cnd[2];
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
								}else{
									n_next = cnd[2];
								}
							}
						}
					}else if(juego.faros[cnd[1]].llave == TRUE){
						//solo llave del 1
						if(df2<df0){
							n_turn = df2+d20+4;
							n_next = cnd[2];
							n_connect = juego.faros_count;
						}else{
							//0 o empate priorizamos conexión rápida
							n_turn = df0+d20+4;
							n_next = cnd[0];
							n_connect = cnd[1];
						}
					}else if(juego.faros[cnd[2]].llave == TRUE){
						//solo llave del 2
						if(df1<df0){
							n_turn = df1+d01+4;
							n_next = cnd[1];
							n_connect = juego.faros_count;
						}else{
							//0 o empate priorizamos conexió rápida
							n_turn = df0+d01+4;
							n_next = cnd[0];
							n_connect = cnd[2];
						}
						
					}else{
						//ninguna llave como si no hubiera nada
						n_connect = juego.faros_count;
						d0 = fmin((df0+d20+d12+d01+6),fmin((df0+d20+d20+d01+6),fmin((df0+d01+d12+d20+6),(df0+d01+d01+d20+6))));
						d1 = fmin((df1+d12+d20+4),(df1+d01+d20+3));
						d2 = fmin((df2+d12+d01+3),(df2+d20+d01+4));
						n_turn = fmin(d0,fmin(d1,d2));
						if(n_turn != d0 && n_turn != d1){
							//al 2
							n_next = cnd[2];
						}else if(n_turn != d1 && n_turn != d2){
							//al 0
							n_next = cnd[0];
						}else if(n_turn =! d2 && n_turn != d0){
							//al 1
							n_next = cnd[1];
						}else if(n_turn != d0){
							// iguales 1 y 2
							if(df1<df2){
								n_next = cnd[1];
							}else if(df2<df1){
								n_next = cnd[2];
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
								}else{
									n_next = cnd[2];
								}
							}
						}else if(n_turn != d1){
							//iguales 0 y 2
							if(df0<df2){
								n_next = cnd[0];
							}else if(df2<df0){
								n_next = cnd[2];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[2];
								}
							}
						}else if(n_turn != d2){
							//iguales 0 y 1
							if(df0<df1){
								n_next = cnd[0];
							}else if(df1<df0){
								n_next = cnd[1];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[1];
								}
							}
						}else{
							//triple empate
							if(df0<df1 && df0<df2){
								n_next = cnd[0];
							}else if(df1<df0 && df1<df2){
								n_next = cnd[1];
							}else if(df2<df0 && df2<df1){
								n_next = cnd[2];
							}else if(df0>df1 && df0>df2){
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
								}else{
									n_next = cnd[2];
								}
							}else if(df1>df0 && df1>df2){
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[2];
								}
							}else if(df2>df0 && df2>df1){
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[1];
								}
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia && juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
								}else if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
								}else{
									n_next = cnd[2];
								}
							}
						}
					}
				}else if(is_conected(cnd[2],juego.faros[cnd[0]])==TRUE){
					//falta 0-1 y 1-2
					df0 = juego.faros[cnd[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					df1 = juego.faros[cnd[1]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					df2 = juego.faros[cnd[2]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					d01 = juego.faros[cnd[0]].distancia[juego.faros[cnd[1]].posicion[0]][juego.faros[cnd[1]].posicion[1]];
					d12 = juego.faros[cnd[1]].distancia[juego.faros[cnd[2]].posicion[0]][juego.faros[cnd[2]].posicion[1]];
					d20 = juego.faros[cnd[2]].distancia[juego.faros[cnd[0]].posicion[0]][juego.faros[cnd[0]].posicion[1]];
					if(juego.faros[cnd[0]].llave == TRUE && juego.faros[cnd[1]].llave == TRUE && juego.faros[cnd[2]].llave == TRUE){
						//las 3 llaves
						d2 = fmin((df2+d12+4),(df2+d20+4));
						d1 = df1+3;
						d0 = fmin((df0+d20+4),(df0+d01+4));
						n_turn = fmin(d2,fmin(d1,d0));
						if(n_turn!= d2 && n_turn != d1){
							//al 0
							n_next = cnd[0];
							n_connect = cnd[1];
						}else if(n_turn != d1 && n_turn != d0){
							//al 2
							n_next = cnd[2];
							n_connect = cnd[1];
						}else if(n_turn != d0 && n_turn != d2){
							//al 1
							n_next = cnd[1];
							//conectar al de más energia
							if(juego.faros[cnd[2]].energia<juego.faros[cnd[0]].energia){
								n_connect = cnd[0];
							}else{
								n_connect = cnd[2];
							}
						}else if(n_turn != d2){
							// 0 y 1 iguales al de menor df
							if(df0<df1){
								n_next = cnd[0];
								n_connect = cnd[1];
							}else if(df1<df0){
								n_next = cnd[1];
								//conectar al de más energia
								if(juego.faros[cnd[2]].energia<juego.faros[cnd[0]].energia){
									n_connect = cnd[0];
								}else{
									n_connect = cnd[2];
								}
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next=cnd[0];
									n_connect = cnd[1];
								}else{
									n_next = cnd[1];
									//conectar al de más energia
									if(juego.faros[cnd[2]].energia<juego.faros[cnd[0]].energia){
										n_connect = cnd[0];
									}else{
										n_connect = cnd[2];
									}
								}
							}
						}else if(n_turn != d1){
							// 0 y 2 iguales al de menor df
							n_connect = cnd[1];
							if(df0<df2){
								n_next = cnd[0];
							}else if(df2<df0){
								n_next = cnd[2];
							}else{
								if(juego.faros[cnd[2]].energia < juego.faros[cnd[0]].energia){
									n_next = cnd[2];
								}else{
									n_next = cnd[0];
								}
							}
						}else if(n_turn != d0){
							// 2 y 1 iguales al de menor df
							if(df1<df2){
								n_next = cnd[1];
								//conectar al de más energia
								if(juego.faros[cnd[2]].energia<juego.faros[cnd[0]].energia){
									n_connect = cnd[0];
								}else{
									n_connect = cnd[2];
								}
							}else if(df2<df1){
								n_next = cnd[2];
								n_connect = cnd[1];
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									//conectar al de más energia
									if(juego.faros[cnd[2]].energia<juego.faros[cnd[0]].energia){
										n_connect = cnd[0];
									}else{
										n_connect = cnd[2];
									}
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}
						}else{
							//triple empate
							if(df0<df1 && df0<df2){
								n_next = cnd[0];
								n_connect = cnd[1];
							}else if(df1<df0 && df1<df2){
								n_next = cnd[1];
								//conectar al de más energia
								if(juego.faros[cnd[2]].energia<juego.faros[cnd[0]].energia){
									n_connect = cnd[0];
								}else{
									n_connect = cnd[2];
								}
							}else if(df2<df0 && df2<df1){
								n_next = cnd[2];
								n_connect = cnd[1];
							}else if(df0>df1 && df0>df2){
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next =cnd[1];
									//conectar al de más energia
									if(juego.faros[cnd[2]].energia<juego.faros[cnd[0]].energia){
										n_connect = cnd[0];
									}else{
										n_connect = cnd[2];
									}
								}else{
									n_next =cnd[2];
									n_connect = cnd[1];
								}
							}else if(df1>df0 && df1>df2){
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[1];
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}else if(df2>df0 && df2>df1){
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
									n_connect = cnd[1];
								}else{
									n_next = cnd[1];
									//conectar al de más energia
									if(juego.faros[cnd[2]].energia<juego.faros[cnd[0]].energia){
										n_connect = cnd[0];
									}else{
										n_connect = cnd[2];
									}
								}
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia && juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[1];
								}else if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									//conectar al de más energia
									if(juego.faros[cnd[2]].energia<juego.faros[cnd[0]].energia){
										n_connect = cnd[0];
									}else{
										n_connect = cnd[2];
									}
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}
						}
					}else if(juego.faros[cnd[0]].llave == TRUE && juego.faros[cnd[1]].llave == TRUE){
						//llaves del 0 y 1
						d0 = fmin((df0+d01+d12+4),(df0+d20+d12+4));
						d1 = df1+d12+4;
						d2 = df2+d12+4;
						n_turn = fmin(d0,fmin(d1,d2));
						if(n_turn != d0 && n_turn != d1){
							//al 2
							n_next = cnd[2];
							n_connect = cnd[1];
						}else if(n_turn != d1 && n_turn != d2){
							//al 0
							n_next = cnd[0];
							n_connect = cnd[1];
						}else if(n_turn != d2 && n_turn != d0){
							//al 1
							n_next = cnd[1];
							n_connect = cnd[0];
						}else if(n_turn != d0){
							//empate 1 y 2
							if(df1<df2){
								n_next = cnd[1];
								n_connect = cnd[0];
							}else if(df2<df1){
								n_next = cnd[2];
								n_connect = cnd[1];
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									n_connect = cnd[0];
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}
						}else if(n_turn != d1){
							//empate 0 y 2
							if(df0<df2){
								n_next = cnd[0];
								n_connect = cnd[1];
							}else if(df2<df0){
								n_next = cnd[2];
								n_connect = cnd[1];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[1];
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}
						}else if(n_turn != d2){
							//empate 0 y 1
							if(df0<df1){
								n_next = cnd[0];
								n_connect = cnd[1];
							}else if(df1<df0){
								n_next = cnd[1];
								n_connect = cnd[0];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
									n_connect = cnd[1];
								}else{
									n_next = cnd[1];
									n_connect = cnd[0];
								}
							}

						}else{
							//triple empate
							if(df0<df1 && df0<df2){
								n_next = cnd[0];
								n_connect = cnd[1];
							}else if(df1<df0 && df1<df2){
								n_next = cnd[1];
								n_connect = cnd[0];
							}else if(df2<df0 && df2<df1){
								n_next = cnd[2];
								n_connect = cnd[1];
							}else if(df0>df1 && df0>df2){
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									n_connect = cnd[0];
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}else if(df1>df0 && df1>df2){
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[1];
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}else if(df2>df0 && df2>df1){
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
									n_connect = cnd[1];
								}else{
									n_next = cnd[1];
									n_connect = cnd[0];
								}
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia && juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[1];
								}else if(juego.faros[cnd[1]].energia <juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									n_connect = cnd[0];
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}
						}
					}else if(juego.faros[cnd[1]].llave == TRUE && juego.faros[cnd[2]].llave == TRUE){
						//llaves del 1 y 2
						d0 = df0+d01+4;
						d1 = df1+d01+4;
						d2 = fmin((df2+d12+d01+4),(df2+d20+d01+4));
						n_turn = fmin(d0,fmin(d1,d2));
						if(n_turn != d0 && n_turn != d1){
							//al 2
							n_next = cnd[2];
							n_connect = cnd[1];
						}else if(n_turn != d1 && n_turn != d2){
							//al 0
							n_next = cnd[0];
							n_connect = cnd[1];
						}else if(n_turn != d2 && n_turn != d0){
							//al 1
							n_next = cnd[1];
							n_connect = cnd[2];
						}else if(n_turn != d0){
							//empate 1 y 2
							if(df1<df2){
								n_next = cnd[1];
								n_connect = cnd[2];
							}else if(df2<df1){
								n_next = cnd[2];
								n_connect = cnd[1];
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									n_connect = cnd[2];
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}
						}else if(n_turn != d1){
							//empate 0 y 2
							if(df0<df2){
								n_next = cnd[0];
								n_connect = cnd[1];
							}else if(df2<df0){
								n_next = cnd[2];
								n_connect = cnd[1];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[1];
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}
						}else if(n_turn != d2){
							//empate 0 y 1
							if(df0<df1){
								n_next = cnd[0];
								n_connect = cnd[1];
							}else if(df1<df0){
								n_next = cnd[1];
								n_connect = cnd[2];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
									n_connect = cnd[1];
								}else{
									n_next = cnd[1];
									n_connect = cnd[2];
								}
							}

						}else{
							//triple empate
							if(df0<df1 && df0<df2){
								n_next = cnd[0];
								n_connect = cnd[1];
							}else if(df1<df0 && df1<df2){
								n_next = cnd[1];
								n_connect = cnd[2];
							}else if(df2<df0 && df2<df1){
								n_next = cnd[2];
								n_connect = cnd[1];
							}else if(df0>df1 && df0>df2){
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									n_connect = cnd[2];
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}else if(df1>df0 && df1>df2){
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[1];
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}else if(df2>df0 && df2>df1){
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
									n_connect = cnd[1];
								}else{
									n_next = cnd[1];
									n_connect = cnd[2];
								}
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia && juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[1];
								}else if(juego.faros[cnd[1]].energia <juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									n_connect = cnd[2];
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}
						}
					}else if(juego.faros[cnd[2]].llave == TRUE && juego.faros[cnd[0]].llave == TRUE){
						//llaves 2 y 0
						n_next = cnd[1];
						n_turn = df1+3;
						if(juego.faros[cnd[2]].energia < juego.faros[cnd[0]].energia){
							n_connect = cnd[0];
						}else{
							n_connect = cnd[2];
						}
					}else if(juego.faros[cnd[0]].llave == TRUE){
						//solo llave del 0
						if(df1<df2){
							n_turn = df1+d12+4;
							n_next = cnd[1];
							n_connect = cnd[0];
						}else if(df2<df1){
							n_turn = df2+d12+4;
							n_next = cnd[2];
							n_connect = juego.faros_count;
						}else{
							//empate
							n_turn = df2+d12+4;
							if(df1<df2){
								n_next = cnd[1];
								n_connect = cnd[0];
							}else if(df2<df1){
								n_next = cnd[2];
								n_connect = juego.faros_count;
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									n_connect = cnd[0];
								}else{
									n_next = cnd[2];
									n_connect = juego.faros_count;
								}
							}
						}
					}else if(juego.faros[cnd[1]].llave == TRUE){
						//solo llave del 1
						d2 = fmin((df2+d12+d01+4),(df2+d20+d01+4));
						d0 = fmin((df0+d01+d12+4),(df0+d20+d12+4));
						n_connect = cnd[1];
						if(d2<d0){
							n_turn = d2;
							n_next = cnd[2];
						}else if(d0<d2){
							n_turn = d0;
							n_next = cnd[0];
						}else{
							n_turn = d2;
							if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
								n_next = cnd[0];
							}else{
								n_next = cnd[2];
							}
						}
					}else if(juego.faros[cnd[2]].llave == TRUE){
						//solo llave del 2
						if(df0<df1){
							n_turn = df0+d01+4;
							n_next = cnd[0];
							n_connect = juego.faros_count;
						}else{
							//1 o empate priorizamos conexió rápida
							n_turn = df1+d01+4;
							n_next = cnd[1];
							n_connect = cnd[2];
						}
						
					}else{
						//ninguna llave como si no hubiera nada
						n_connect = juego.faros_count;
						d0 = fmin((df0+d20+d12+3),(df0+d01+d12+4));
						d1 = fmin((df1+d12+d20+d01+6),fmin((df1+d12+d12+d01+6),fmin((df1+d01+d20+d12+6),(df1+d01+d01+d12+6))));
						d2 = fmin((df2+d12+d01+4),(df2+d20+d01+3));
						n_turn = fmin(d0,fmin(d1,d2));
						if(n_turn != d0 && n_turn != d1){
							//al 2
							n_next = cnd[2];
						}else if(n_turn != d1 && n_turn != d2){
							//al 0
							n_next = cnd[0];
						}else if(n_turn =! d2 && n_turn != d0){
							//al 1
							n_next = cnd[1];
						}else if(n_turn != d0){
							// iguales 1 y 2
							if(df1<df2){
								n_next = cnd[1];
							}else if(df2<df1){
								n_next = cnd[2];
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
								}else{
									n_next = cnd[2];
								}
							}
						}else if(n_turn != d1){
							//iguales 0 y 2
							if(df0<df2){
								n_next = cnd[0];
							}else if(df2<df0){
								n_next = cnd[2];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[2];
								}
							}
						}else if(n_turn != d2){
							//iguales 0 y 1
							if(df0<df1){
								n_next = cnd[0];
							}else if(df1<df0){
								n_next = cnd[1];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[1];
								}
							}
						}else{
							//triple empate
							if(df0<df1 && df0<df2){
								n_next = cnd[0];
							}else if(df1<df0 && df1<df2){
								n_next = cnd[1];
							}else if(df2<df0 && df2<df1){
								n_next = cnd[2];
							}else if(df0>df1 && df0>df2){
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
								}else{
									n_next = cnd[2];
								}
							}else if(df1>df0 && df1>df2){
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[2];
								}
							}else if(df2>df0 && df2>df1){
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[1];
								}
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia && juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
								}else if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
								}else{
									n_next = cnd[2];
								}
							}
						}
					}
				}else{
					//faltan las 3
					df0 = juego.faros[cnd[0]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					df1 = juego.faros[cnd[1]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					df2 = juego.faros[cnd[2]].distancia[jugador.posicion[0]][jugador.posicion[1]];
					d01 = juego.faros[cnd[0]].distancia[juego.faros[cnd[1]].posicion[0]][juego.faros[cnd[1]].posicion[1]];
					d12 = juego.faros[cnd[1]].distancia[juego.faros[cnd[2]].posicion[0]][juego.faros[cnd[2]].posicion[1]];
					d20 = juego.faros[cnd[2]].distancia[juego.faros[cnd[0]].posicion[0]][juego.faros[cnd[0]].posicion[1]];
					if(juego.faros[cnd[0]].llave == TRUE && juego.faros[cnd[1]].llave == TRUE && juego.faros[cnd[2]].llave == TRUE){
						//las 3 llaves
						d0 = fmin((df0+d20+5),(df0+d01+5));
						d1 = fmin((df1+d01+5),(df1+d12+5));
						d2 = fmin((df2+d20+5),(df2+d12+5));
						n_turn = fmin(d0,fmin(d1,d2));
						if(n_turn != d0 && n_turn != d1){
							//al 2
							n_next = cnd[2];
							//conectar al de más energia
							if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
								n_connect = cnd[1];
							}else{
								n_connect = cnd[0];
							}
						}else if(n_turn != d1 && n_turn != d2){
							//al 0
							n_next = cnd[0];
							//conectar al de más energia
							if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
								n_connect = cnd[2];
							}else{
								n_connect = cnd[1];
							}
						}else if(n_turn != d0 && n_turn != d1){
							//al 1
							n_next = cnd[1];
							//conectar al de más energia
							if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
								n_connect = cnd[2];
							}else{
								n_connect = cnd[0];
							}
						}else if(n_turn != d0){
							// 1 y 2 iguales al menor df
							if(df1<df2){
								n_next = cnd[1];
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_connect = cnd[2];
								}else{
									n_connect = cnd[0];
								}
							}else if(df2<df1){
								n_next = cnd[2];
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_connect = cnd[1];
								}else{
									n_connect = cnd[0];
								}
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
										n_connect = cnd[2];
									}else{
										n_connect = cnd[0];
									}
								}else{
									n_next = cnd[2];
									if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
										n_connect = cnd[1];
									}else{
										n_connect = cnd[0];
									}
								}
							}
						}else if(n_turn != d1){
							// 0 y 2 iguales
							if(df0<df2){
								n_next = cnd[0];
								if(juego.faros[cnd[2]].energia < juego.faros[cnd[1]].energia){
									n_connect = cnd[1];
								}else{
									n_connect = cnd[2];
								}
							}else if(df2<df0){
								n_next = cnd[2];
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_connect = cnd[1];
								}else{
									n_connect = cnd[0];
								}
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
										n_connect = cnd[2];
									}else{
										n_connect = cnd[1];
									}
								}else{
									n_next = cnd[2];
									if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
										n_connect = cnd[1];
									}else{
										n_connect = cnd[0];
									}
								}
							}
						}else if(n_turn != d2){
							// 0 y 1 iguales
							if(df0<df1){
								n_next = cnd[0];
								if(juego.faros[cnd[2]].energia < juego.faros[cnd[1]].energia){
									n_connect = cnd[1];
								}else{
									n_connect = cnd[2];
								}
							}else if(df1<df0){
								n_next = cnd[1];
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_connect = cnd[2];
								}else{
									n_connect = cnd[0];
								}
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
									if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
										n_connect = cnd[2];
									}else{
										n_connect = cnd[1];
									}
								}else{
									n_next = cnd[1];
									if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
										n_connect = cnd[2];
									}else{
										n_connect = cnd[0];
									}
								}
							}
						}else{
							//triple empate
							if(df0<df1 && df0<df2){
								n_next = cnd[0];
								if(juego.faros[cnd[2]].energia < juego.faros[cnd[1]].energia){
									n_connect = cnd[1];
								}else{
									n_connect = cnd[2];
								}
							}else if(df1<df0 && df1<df2){
								n_next = cnd[1];
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_connect = cnd[2];
								}else{
									n_connect = cnd[0];
								}
							}else if(df2<df0 && df2<df1){
								n_next = cnd[2];
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_connect = cnd[1];
								}else{
									n_connect = cnd[0];
								}
							}else if(df0>df1 && df0>df2){
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
										n_connect = cnd[2];
									}else{
										n_connect = cnd[0];
									}
								}else{
									n_next = cnd[2];
									if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
										n_connect = cnd[1];
									}else{
										n_connect = cnd[0];
									}
								}
							}else if(df1>df0 && df1>df2){
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
										n_connect = cnd[2];
									}else{
										n_connect = cnd[1];
									}
								}else{
									n_next = cnd[2];
									if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
										n_connect = cnd[1];
									}else{
										n_connect = cnd[0];
									}
								}
							}else if(df2>df0 && df2>df1){
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[0]].energia){
									n_next = cnd[1];
									if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
										n_connect = cnd[2];
									}else{
										n_connect = cnd[0];
									}
								}else{
									n_next = cnd[0];
									if(juego.faros[cnd[2]].energia < juego.faros[cnd[1]].energia){
										n_connect = cnd[1];
									}else{
										n_connect = cnd[2];
									}
								}
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia && juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									if(juego.faros[cnd[2]].energia < juego.faros[cnd[1]].energia){
										n_connect = cnd[1];
									}else{
										n_connect = cnd[2];
									}
								}else if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
										n_connect = cnd[2];
									}else{
										n_connect = cnd[0];
									}
								}else{
									n_next = cnd[2];
									if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
										n_connect = cnd[1];
									}else{
										n_connect = cnd[0];
									}
								}
							}
						}
					}else if(juego.faros[cnd[0]].llave == TRUE && juego.faros[cnd[1]].llave == TRUE){
						//llaves 0 y 1
						d0 = fmin((df0+d20+d12+6),(df0+d01+d12+6));
						d1 = fmin((df1+d12+d20+6),(df1+d01+d20+6));
						d2 = fmin((df2+d12+5),(df2+d20+5));
						n_turn = fmin(d0,fmin(d1,d2));
						if(n_turn != d1 && n_turn != d2){
							//al 0
							n_next = cnd[0];
							n_connect = cnd[1];
						}else if(n_turn != d0 && n_turn != d2){
							//al 1
							n_next = cnd[1];
							n_connect = cnd[0];
						}else if(n_turn != d1 && n_turn != d0){
							//al 2
							n_next = cnd[2];
							//conectar el de más energia
							if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
								n_connect = cnd[1];
							}else{
								n_connect = cnd[0];
							}
						}else if(n_turn != d0){
							//empate 1 y 2
							if(df1<df2){
								n_next = cnd[1];
								n_connect = cnd[0];
							}else if(df2<df1){
								n_next = cnd[2];
								//conectar el de más energia
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_connect = cnd[1];
								}else{
									n_connect = cnd[0];
								}
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									n_connect = cnd[0];
								}else{
									n_next = cnd[2];
									//conectar el de más energia
									if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
										n_connect = cnd[1];
									}else{
										n_connect = cnd[0];
									}
								}
							}
						}else if(n_turn != d1){
							//empate 0 y 2
							if(df0<df2){
								n_next = cnd[0];
								n_connect = cnd[1];
							}else if(df2<df0){
								n_next = cnd[2];
								//conectar el de más energia
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_connect = cnd[1];
								}else{
									n_connect = cnd[0];
								}
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[1];
								}else{
									n_next = cnd[2];
									//conectar el de más energia
									if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
										n_connect = cnd[1];
									}else{
										n_connect = cnd[0];
									}
								}
							}
						}else if(n_turn != d2){
							if(df1<df0){
								n_next = cnd[1];
								n_connect = cnd[0];
							}else if(df0<df1){
								n_next = cnd[0];
								n_connect = cnd[1];
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[0]].energia){
									n_next = cnd[1];
									n_connect = cnd[0];
								}else{
									n_next = cnd[0];
									n_connect = cnd[1];
								}
							}
						}else{
							if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia && juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
								n_next = cnd[0];
								n_connect = cnd[1];
							}else if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
								n_next = cnd[1];
								n_connect = cnd[0];
							}else{
								n_next = cnd[2];
								//conectar el de más energia
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_connect = cnd[1];
								}else{
									n_connect = cnd[0];
								}
							}
						}
					}else if(juego.faros[cnd[0]].llave == TRUE && juego.faros[cnd[2]].llave == TRUE){
						//llave 0 y 2
						d0 = fmin((df0+d20+d12+6),(df0+d01+d12+6));
						d1 = fmin((df1+d12+d20+6),(df1+d01+d20+6));
						d2 = fmin((df2+d12+5),(df2+d20+5));
						n_turn = fmin(d0,fmin(d1,d2));
						if(n_turn != d1 && n_turn != d2){
							//al 0
							n_next = cnd[0];
							n_connect = cnd[2];
						}else if(n_turn != d0 && n_turn != d2){
							//al 1
							n_next = cnd[1];
							//conectar el de más energia
							if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
								n_connect = cnd[2];
							}else{
								n_connect = cnd[0];
							}
						}else if(n_turn != d1 && n_turn != d0){
							//al 2
							n_next = cnd[2];
							n_connect = cnd[0];
						}else if(n_turn != d0){
							//empate 1 y 2
							if(df1<df2){
								n_next = cnd[1];
								//conectar el de más energia
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_connect = cnd[2];
								}else{
									n_connect = cnd[0];
								}
							}else if(df2<df1){
								n_next = cnd[2];
								n_connect = cnd[0];
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									//conectar el de más energia
									if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
										n_connect = cnd[2];
									}else{
										n_connect = cnd[0];
									}
								}else{
									n_next = cnd[2];
									n_connect = cnd[0];
								}
							}
						}else if(n_turn != d1){
							//empate 0 y 2
							if(df0<df2){
								n_next = cnd[0];
								n_connect = cnd[2];
							}else if(df2<df0){
								n_next = cnd[2];
								n_connect = cnd[0];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									n_connect = cnd[2];
								}else{
									n_next = cnd[2];
									n_connect = cnd[0];
								}
							}
						}else if(n_turn != d2){
							if(df1<df0){
								n_next = cnd[1];
								//conectar el de más energia
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_connect = cnd[2];
								}else{
									n_connect = cnd[0];
								}
							}else if(df0<df1){
								n_next = cnd[0];
								n_connect = cnd[2];
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[0]].energia){
									n_next = cnd[1];
									//conectar el de más energia
									if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
										n_connect = cnd[2];
									}else{
										n_connect = cnd[0];
									}
								}else{
									n_next = cnd[0];
									n_connect = cnd[2];
								}
							}
						}else{
							if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia && juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
								n_next = cnd[0];
								n_connect = cnd[2];
							}else if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
								n_next = cnd[1];
								//conectar el de más energia
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_connect = cnd[2];
								}else{
									n_connect = cnd[0];
								}
							}else{
								n_next = cnd[2];
								n_connect = cnd[0];
							}
						}
					}else if(juego.faros[cnd[1]].llave == TRUE && juego.faros[cnd[2]].llave == TRUE){
						//llaves 1 y 2
						d0 = fmin((df0+d20+d12+6),(df0+d01+d12+6));
						d1 = fmin((df1+d12+d20+6),(df1+d01+d20+6));
						d2 = fmin((df2+d12+5),(df2+d20+5));
						n_turn = fmin(d0,fmin(d1,d2));
						if(n_turn != d1 && n_turn != d2){
							//al 0
							n_next = cnd[0];
							//conectar el de más energia
							if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
								n_connect = cnd[2];
							}else{
								n_connect = cnd[1];
							}
						}else if(n_turn != d0 && n_turn != d2){
							//al 1
							n_next = cnd[1];
							n_connect = cnd[2];
						}else if(n_turn != d1 && n_turn != d0){
							//al 2
							n_next = cnd[2];
							n_connect = cnd[1];
						}else if(n_turn != d0){
							//empate 1 y 2
							if(df1<df2){
								n_next = cnd[1];
								n_connect = cnd[2];
							}else if(df2<df1){
								n_next = cnd[2];
								n_connect = cnd[1];
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
									n_connect = cnd[2];
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}
						}else if(n_turn != d1){
							//empate 0 y 2
							if(df0<df2){
								n_next = cnd[0];
								//conectar el de más energia
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_connect = cnd[2];
								}else{
									n_connect = cnd[1];
								}
							}else if(df2<df0){
								n_next = cnd[2];
								n_connect = cnd[1];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
									//conectar el de más energia
									if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
										n_connect = cnd[2];
									}else{
										n_connect = cnd[1];
									}
								}else{
									n_next = cnd[2];
									n_connect = cnd[1];
								}
							}
						}else if(n_turn != d2){
							if(df1<df0){
								n_next = cnd[1];
								n_connect = cnd[2];
							}else if(df0<df1){
								n_next = cnd[0];
								//conectar el de más energia
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_connect = cnd[2];
								}else{
									n_connect = cnd[1];
								}
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[0]].energia){
									n_next = cnd[1];
									n_connect = cnd[2];
								}else{
									n_next = cnd[0];
									//conectar el de más energia
									if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
										n_connect = cnd[2];
									}else{
										n_connect = cnd[1];
									}
								}
							}
						}else{
							if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia && juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
								n_next = cnd[0];
								//conectar el de más energia
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_connect = cnd[2];
								}else{
									n_connect = cnd[1];
								}
							}else if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
								n_next = cnd[1];
								n_connect = cnd[2];
							}else{
								n_next = cnd[2];
								n_connect = cnd[1];
							}
						}
					}else if(juego.faros[cnd[0]].llave == TRUE){
						//solo llave 0
						n_connect = cnd[0];
						d1 = fmin((df1+d12+d20+6),(df1+d01+d20+6));
						d2 = fmin((df2+d12+d01+6),(df2+d20+d01+6));
						if(d1<d2){
							n_turn = d1;
							n_next = cnd[1];
						}else if(d2<d1){
							n_turn = d2;
							n_next = cnd[2];
						}else{
							n_turn = d1;
							if(df1<df2){
								n_next = cnd[1];
							}else if(df2<df1){
								n_next = cnd[2];
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
								}else{
									n_next = cnd[2];
								}
							}
						}
					}else if(juego.faros[cnd[1]].llave == TRUE){
						//solo llave 1
						n_connect = cnd[1];
						d0 = fmin((df0+d01+d12+6),(df0+d20+d12+6));
						d2 = fmin((df2+d20+d01+6),(df2+d12+d01+6));
						if(d0<d2){
							n_turn = d0;
							n_next = cnd[0];
						}else if(d2<d0){
							n_turn = d2;
							n_next = cnd[2];
						}else{
							n_turn = d0;
							if(df0<df2){
								n_next = cnd[0];
							}else if(df2<df0){
								n_next = cnd[2];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[2];
								}
							}
						}
					}else if(juego.faros[cnd[2]].llave == TRUE){
						//solo llave 2
						n_connect = cnd[2];
						d0 = fmin((df0+d01+d12+6),(df0+d20+d12+6));
						d1 = fmin((df1+d12+d20+6),(df1+d01+d20+6));
						if(d0<d1){
							n_turn = d0;
							n_next = cnd[0];
						}else if(d1<d0){
							n_turn = d1;
							n_next = cnd[1];
						}else{
							n_turn = d0;
							if(df0<df1){
								n_next = cnd[0];
							}else if(df1<df0){
								n_next = cnd[1];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[1];
								}
							}
						}
					}else{
						//ninguna llave
						d0 = fmin((df0+d01+d12+d20+6),fmin((df0+d01+d01+d20+6),fmin((df0+d20+d12+d01+6),(df0+d20+d20+d01+6))));
						d1 = fmin((df1+d01+d20+d12+6),fmin((df1+d01+d01+d12+6),fmin((df1+d12+d20+d01+6),(df1+d12+d12+d01+6))));
						d2 = fmin((df2+d12+d01+d20+6),fmin((df2+d12+d12+d20+6),fmin((df2+d20+d01+d12+6),(df2+d20+d20+d12+6))));
						n_turn = fmin(d0,fmin(d1,d2));
						n_connect = juego.faros_count;
						if(n_turn != d1 && n_turn != d2){
							n_next = cnd[0];
						}else if(n_turn != d0 && n_turn != d2){
							n_next = cnd[1];
						}else if(n_turn != d0 && n_turn != d1){
							n_next = cnd[2];
						}else if(n_turn != d0){
							//empate 1 y2
							if(df1 < df2){
								n_next = cnd[1];
							}else if(df2 < df1){
								n_next = cnd[2];
							}else{
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
								}else{
									n_next = cnd[2];
								}
							}
						}else if(n_turn != d1){
							//empate 0 y 2
							if(df0 < df2){
								n_next = cnd[0];
							}else if(df2 < df0){
								n_next = cnd[2];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[2];
								}
							}
						}else if(n_turn != d2){
							//empate 0 y 1
							if(df0<df1){
								n_next = cnd[0];
							}else if(df1<df0){
								n_next = cnd[1];
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[1];
								}
							}
						}else{
							if(df0<df1 && df0<df2){
								n_next = cnd[0];
							}else if(df1<df0 && df1<df2){
								n_next = cnd[1];
							}else if(df2<df0 && df2<df1){
								n_next = cnd[2];
							}else if(df0>df1 && df0>df2){
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
								}else{
									n_next = cnd[2];
								}
							}else if(df1>df0 && df1>df2){
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[0];
								}else{
									n_next = cnd[2];
								}
							}else if(df2>df0 && df2>df1){
								if(juego.faros[cnd[1]].energia < juego.faros[cnd[0]].energia){
									n_next = cnd[1];
								}else{
									n_next = cnd[0];
								}
							}else{
								if(juego.faros[cnd[0]].energia < juego.faros[cnd[1]].energia && juego.faros[cnd[0]].energia <juego.faros[cnd[2]].energia){
									n_next = cnd[0];
								}else if(juego.faros[cnd[1]].energia < juego.faros[cnd[2]].energia){
									n_next = cnd[1];
								}else{
									n_next = cnd[2];
								}
							}
						}
					}
				}
			}
			break;
		}
		//snprintf(msg,MAX_CANON,"comparacion %u",n_turn);
		//snd_log(msg);
		//se llega con n_turn,n_next,n_connect COMPARACION
		if(n_turn != (juego.maxx*juego.maxy)){
			//snd_log("dentro");
			//devemos tenerlo en cuenta
			//calcular energia
			nxt_dist = juego.faros[n_next].distancia[jugador.posicion[0]][jugador.posicion[1]];
			if((nxt_dist*E_LOSS)>=juego.triangulos[i].energia){
				//estará vacio al llegar
					ener = juego.triangulos[i].energia;
			}else{
				if(juego.faros[n_next].dueno == juego.player_num){
					//ya es nuestro
					ener = juego.triangulos[i].energia - juego.faros[n_next].energia +(nxt_dist*E_LOSS);
					if(ener<0){
						//snd_log("nuestro negativo");
						ener = 0;
					}else{
						ener = ener;
					}
				}else if(juego.faros[n_next].dueno > 0){
					//es de otro
					ener = juego.triangulos[i].energia + juego.faros[n_next].energia -(nxt_dist*E_LOSS);
					if(ener<0){
						//snd_log("otro negativo");
						ener = 0;
					}else{
						//snprintf(msg,MAX_CANON,"otro dueno faro: %u, ener: %u", juego.faros[n_next].energia, ener);
						//snd_log(msg);
						ener = ener;
					}
				}else{
					//está vacio
					ener = juego.triangulos[i].energia;
				}
			}
			if(controlados = 3){
				n_ener = ener;
			}else{
				n_ener = juego.triangulos[i].energia*(3-controlados);
			}
			//snprintf(msg,MAX_CANON,"check ener jugador: %u objetivo: %u ener: %u",jugador.energia, obj.energy, ener);
			//snd_log(msg);
			if(jugador.energia > n_ener || jugador.energia <= obj.n_energy || to==juego.tri_count){
				//no tener en cuenta para evitar movimientos por energia.
				//snd_log("dentro");
				if(check_tri_cross(juego,i)==FALSE){
					//no hay cruces
					if(to==juego.tri_count){
						//el primero adoptar
						//snprintf(msg,MAX_CANON,"cambio1 tri %u tri_ener %u ener = %u",i,juego.triangulos[i].energia,ener);
						//snd_log(msg);
						t_tri = i;
						t_turn = n_turn;
						to = i;
						obj.next = n_next;
						obj.connect = n_connect;
						obj.tri = t_tri;
						obj.energy = ener;
						obj.n_energy = n_ener;
					}else{
						//ya hay uno, comparar
						if(check_tri(juego,t_turn,n_turn,t_tri,i) == TRUE){
							//snprintf(msg,MAX_CANON,"cambio2 tri %u tri_ener %u ener = %u",i,juego.triangulos[i].energia,ener);
							//snd_log(msg);
							//cambiar
							t_tri = i;
							t_turn = n_turn;
							obj.next = n_next;
							obj.connect = n_connect;
							obj.tri = t_tri;
							obj.energy = ener;
							obj.n_energy = n_ener;
						}
					}
				}
			}
		}
	}
	return obj;	
}

short check_tri(struct game juego, unsigned int t_turn, unsigned int n_turn, unsigned short t_tri, unsigned short n_tri){
	unsigned int t_points = juego.triangulos[t_tri].puntos;
	unsigned int n_points = juego.triangulos[n_tri].puntos;
	unsigned int dt;
	unsigned int dp;
	if(n_points > t_points){
		//el nuevo más puntos
		if(n_turn <= t_turn){
			//snprintf(msg,MAX_CANON,"%u CT: T t_turn: %u t_points: %u n_turn: %u n_points: %u",n_tri,t_turn,t_points,n_turn,n_points);
			//snd_log(msg);
			return TRUE;
		}else{
			//el vijeo menos turnos
			dt = n_turn - t_turn;
			dp = dt*t_points;
			if (dp>=n_points){
				//snprintf(msg,MAX_CANON,"%u CT: F t_turn: %u t_points: %u n_turn: %u n_points: %u",n_tri,t_turn,t_points,n_turn,n_points);
				//snd_log(msg);
				return FALSE;
			}else{
				//snprintf(msg,MAX_CANON,"%u CT: T t_turn: %u t_points: %u n_turn: %u n_points: %u",n_tri,t_turn,t_points,n_turn,n_points);
				//snd_log(msg);
				return TRUE;
			}
		}
	}else if(t_points > n_points){
		//el viejo más puntos
		if(t_turn <= n_turn){
			//snprintf(msg,MAX_CANON,"%u CT: F t_turn: %u t_points: %u n_turn: %u n_points: %u",n_tri,t_turn,t_points,n_turn,n_points);
			//snd_log(msg);
			return FALSE;
		}else{
			dt = t_turn - n_turn;
			dp = dt*n_points;
			if(dp>=t_points){
				//snprintf(msg,MAX_CANON,"%u CT: T t_turn: %u t_points: %u n_turn: %u n_points: %u",n_tri,t_turn,t_points,n_turn,n_points);
				//snd_log(msg);
				return TRUE;
			}else{
				//snprintf(msg,MAX_CANON,"%u CT: F t_turn: %u t_points: %u n_turn: %u n_points: %u",n_tri,t_turn,t_points,n_turn,n_points);
				//snd_log(msg);
				return FALSE;
			}
		}
		
	}else{
		//mismos puntos
		if(t_turn>n_turn){
			//snprintf(msg,MAX_CANON,"%u CT: T t_turn: %u t_points: %u n_turn: %u n_points: %u",n_tri,t_turn,n_points,n_turn,n_points);
			//snd_log(msg);
			return TRUE;
		}else if(n_turn < t_turn){
			//snprintf(msg,MAX_CANON,"%u CT: F t_turn: %u t_points: %u n_turn: %u n_points: %u",n_tri,t_turn,n_points,n_turn,n_points);
			//snd_log(msg);
			return FALSE;
		}else{
			//empate en turnos y puntos
			if(juego.triangulos[t_tri].energia < juego.triangulos[n_tri].energia){
				//snprintf(msg,MAX_CANON,"%u CT: F t_turn: %u t_points: %u n_turn: %u n_points: %u",n_tri,t_turn,t_points,n_turn,n_points);
				//snd_log(msg);
				return FALSE;
			}else{
				//snprintf(msg,MAX_CANON,"%u CT: T t_turn: %u t_points: %u n_turn: %u n_points: %u",n_tri,t_turn,t_points,n_turn,n_points);
				//snd_log(msg);
				return TRUE;
			}
		}
	}
}

short check_tri_cross(struct game juego,unsigned short tri){
	if(check_cross(juego,juego.triangulos[tri].faros[0],juego.triangulos[tri].faros[1]) == TRUE){
		return TRUE;
	}

	if(check_cross(juego,juego.triangulos[tri].faros[1],juego.triangulos[tri].faros[2]) == TRUE){
		return TRUE;
	}

	if(check_cross(juego,juego.triangulos[tri].faros[2],juego.triangulos[tri].faros[0]) == TRUE){
		return TRUE;
	}

	return FALSE;
}

short check_cross(struct game juego, unsigned short f1, unsigned short f2){
	//comprobar si conectar dos faros cruzaría con conexiones
	unsigned short i,j;
	for(i=0;i<juego.faros_count;i++){
		for(j=0;j<juego.faros[i].n_conexiones;j++){
			//if(i!=f1 && i!=f2 && j!=f1 && j!=f2){
				if(do_intersect(juego.faros[i].posicion,juego.faros[juego.faros[i].conexiones[j]].posicion,
					juego.faros[f1].posicion,juego.faros[f2].posicion) == TRUE){
					return TRUE;
				}
			//}
		}
	}
	return FALSE;
}

unsigned short get_lightid(unsigned short y, unsigned short x,struct game *juego){
	unsigned short j;
	for(j=0;j<juego->faros_count;j++){
		if(juego->faros[j].posicion[1] == x && juego->faros[j].posicion[0] == y){
			return j;
		}
	}
	return -1;
}

short is_conected(unsigned short v,struct faro f){
	unsigned short i;
	for(i=0;i<f.n_conexiones;i++){
		if(v==f.conexiones[i]){
			return TRUE;
		}
	}
	return FALSE;
}


int triangle_point(unsigned short t1[2], unsigned short t2[2], unsigned short t3[2],struct game *juego){
	unsigned short maxx = fmax(t1[1],fmax(t2[1],t3[1]));
	unsigned short maxy = fmax(t1[0],fmax(t2[0],t3[0]));
	unsigned short minx = fmin(t1[1],fmin(t2[1],t3[1]));
	unsigned short miny = fmin(t1[0],fmin(t2[0],t3[0]));
	unsigned short y,x;
	unsigned short point=0;
	unsigned short p[2];

/*
	fprintf(stderr,"t1 [%u,%u]\nt2 [%u,%u]\nt3 [%u,%u]\n",t1[0],t1[1],t2[0],t2[1],t3[0],t3[1]);
	fprintf(stderr,"maxy: %u\n",maxy);
	fprintf(stderr,"maxx: %u\n",maxx);
	fprintf(stderr,"miny: %u\n",miny);
	fprintf(stderr,"minx: %u\n",minx);
	
*/
	for(y=miny;y<=maxy;y++){
		for(x=minx;x<=maxx;x++){
			p[0] = y;
			p[1] = x;
			if(juego->map[y][x] ==1){
				if(is_inside(t1,t2,t3,p)==TRUE){
					point++;
				}
			}
		}
	}
	return point;
}

short is_inside(unsigned short t1[2], unsigned short t2[2], unsigned short t3[2], unsigned short p[2]){
	unsigned short maxx = fmax(t1[1],fmax(t2[1],fmax(t3[1],p[1])))+1;
	unsigned short tp[2];
	tp[0] = p[0];
	tp[1] = maxx;
	short c = 0;

	//comprobar t1 con t2
	if(do_intersect(t1,t2,p,tp)){
		if(orientation(t1,p,t2) == 0){
			return on_segment(t1,p,t2);
		}
		c++;
	}

	//comprobar t2 con t3
	if(do_intersect(t2,t3,p,tp)){
		if(orientation(t2,p,t3) == 0){
			return on_segment(t2,p,t3);
		}
		c++;
	}

	//comprobar t3 con t1
	if(do_intersect(t3,t1,p,tp)){
		if(orientation(t3,p,t1) == 0){
			return on_segment(t3,p,t1);
		}
		c++;
	}
	
	if(c==1){
		return TRUE;
	}

	return FALSE;
}


//p1q1 intersec p2q2
short do_intersect(unsigned short p1[2],unsigned short q1[2],unsigned short p2[2],unsigned short q2[2]){
	if((p1[0] == p2[0] && p1[1] == p2[1]) || (p1[0] == q2[0] && p1[1] == q2[1]) || (q1[0] == p2[0] && q1[1] == p2[1]) || (q1[0] == q2[0] && q1[1] == q2[1])){
		return FALSE;
	}

	//Find for orientations needed
	short o1 = orientation(p1,q1,p2);
	short o2 = orientation(p1,q1,q2);
	short o3 = orientation(p2,q2,p1);
	short o4 = orientation(p2,q2,q1);

	//general
	if(o1 != o2 && o3 != o4){
		return TRUE;
	}

	//Special Cases
	if(o1 == 0 && on_segment(p1,q1,p2)==TRUE){return TRUE;}
	if(o2 == 0 && on_segment(p1,q1,q2)==TRUE){return TRUE;}
	if(o3 == 0 && on_segment(p2,q2,p1)==TRUE){return TRUE;}
	if(o4 == 0 && on_segment(p2,q2,q1)==TRUE){return TRUE;}

	return FALSE;
}

//To find orientation of ordered triple (p,q,r)
//The funciotn returns:
//0 -->p, q and r are collinear
//1 -->Clockwise
//2 -->CounterClockwise
short orientation(unsigned short p[2],unsigned short q[2], unsigned short r[2]){
	int val = (q[0] - p[0]) * (r[1] - q[1]) - (q[1] - p[1]) * (r[0] - q[0]);
	if(val==0){
		return 0;
	}else{
		if(val>0){
			return 1;
		}else{
			return 2;
		}
	}
}

//1-true
//0-false
//given q1-q2-p collinear check if p is inside q1,q2
short on_segment(unsigned short q1[2], unsigned short q2[2], unsigned short p[2]){
	if(p[1]<= fmax(q1[1],q2[1]) && p[1] >= fmin(q1[1],q2[1]) && 
		p[0] >= fmax(q1[0],q2[0]) && p[0] >= fmin(q1[0],q2[0])){
		return 	TRUE;
	}
	return FALSE;
}

unsigned int **flood_dist(unsigned short **map,unsigned short maxx, unsigned short maxy, unsigned short pos[2]){
	//devuelve el mapa de distancia (en turnos) a la posición dada
	unsigned int **temp_arr;
	unsigned short i,j;
	unsigned int loop = 1;
	int **arr_act;
	int **arr_nxt;
	int **bck_nxt;
	unsigned short arr_act_len = 1;
	unsigned short arr_nxt_len = 0;
	int act[2];
	int **checked;
	int **bck_chk;
	unsigned int checked_len=1;

	temp_arr = malloc(maxy* sizeof(unsigned int *));
	for(i=0;i<maxy;i++){
		temp_arr[i] = malloc(maxx*sizeof(unsigned int));
		for(j=0;j<maxx;j++){
			temp_arr[i][j] = maxx*maxy;
		}
	}
	temp_arr[pos[0]][pos[1]] = 0;

	arr_act = malloc(sizeof(int *));
	arr_act[0] = malloc(1*sizeof(int));
	arr_act[0][0] = pos[0];
	arr_act[0][1] = pos[1];

	checked = malloc(sizeof(int *));
	checked[0] = malloc(1*sizeof(int));
	checked[0][0] = pos[0];
	checked[0][1] = pos[1];

	do{
		arr_nxt_len = 0;
		for(i=0;i<arr_act_len;i++){
			//234
			//1*5
			//076

			//Dir 0
			act[0] = arr_act[i][0]-1;
			act[1] = arr_act[i][1]-1;
			if(act[0]>0 && act[1]>0){
				if(map[act[0]][act[1]] == 1 && check_in(act,checked,checked_len) == -1){
				
					bck_nxt = arr_nxt;
					arr_nxt = malloc((arr_nxt_len+1)*sizeof(int *));
					if(arr_nxt_len>0){
						for(j=0;	j<=arr_nxt_len;j++){
							arr_nxt[j] = bck_nxt[j];
						}
						free(bck_nxt);
					}
					arr_nxt[arr_nxt_len] = malloc(2*sizeof(int));
					arr_nxt[arr_nxt_len][0] = act[0];
					arr_nxt[arr_nxt_len][1] = act[1];
					arr_nxt_len++;
					temp_arr[act[0]][act[1]] = loop;
					bck_chk = checked;
					checked = malloc((checked_len+1)*sizeof(int *));
					if(checked_len >0){
						for(j=0;j<=checked_len;j++){
							checked[j] = bck_chk[j];
						}
						free(bck_chk);
					}
					checked[checked_len] = malloc(2*sizeof(int));
					checked[checked_len][0] = act[0];
					checked[checked_len][1] = act[1];
					checked_len++;
				}
			}

			//Dir 1
			act[0] = arr_act[i][0];
			act[1] = arr_act[i][1]-1;
			if(act[1]>0){
				if(map[act[0]][act[1]] == 1 && check_in(act,checked,checked_len) == -1){
				
					bck_nxt = arr_nxt;
					arr_nxt = malloc((arr_nxt_len+1)*sizeof(int *));
					if(arr_nxt_len>0){
						for(j=0;	j<=arr_nxt_len;j++){
							arr_nxt[j] = bck_nxt[j];
						}
						free(bck_nxt);
					}
					arr_nxt[arr_nxt_len] = malloc(2*sizeof(int));
					arr_nxt[arr_nxt_len][0] = act[0];
					arr_nxt[arr_nxt_len][1] = act[1];
					arr_nxt_len++;
					temp_arr[act[0]][act[1]] = loop;
					bck_chk = checked;
					checked = malloc((checked_len+1)*sizeof(int *));
					if(checked_len >0){
						for(j=0;j<=checked_len;j++){
							checked[j] = bck_chk[j];
						}
						free(bck_chk);
					}
					checked[checked_len] = malloc(2*sizeof(int));
					checked[checked_len][0] = act[0];
					checked[checked_len][1] = act[1];
					checked_len++;
				}
			}

			//Dir 2
			act[0] = arr_act[i][0]+1;
			act[1] = arr_act[i][1]-1;
			if(act[0]<maxy && act[1]>0){
				if(map[act[0]][act[1]] == 1 && check_in(act,checked,checked_len) == -1){
				
					bck_nxt = arr_nxt;
					arr_nxt = malloc((arr_nxt_len+1)*sizeof(int *));
					if(arr_nxt_len>0){
						for(j=0;	j<=arr_nxt_len;j++){
							arr_nxt[j] = bck_nxt[j];
						}
						free(bck_nxt);
					}
					arr_nxt[arr_nxt_len] = malloc(2*sizeof(int));
					arr_nxt[arr_nxt_len][0] = act[0];
					arr_nxt[arr_nxt_len][1] = act[1];
					arr_nxt_len++;
					temp_arr[act[0]][act[1]] = loop;
					bck_chk = checked;
					checked = malloc((checked_len+1)*sizeof(int *));
					if(checked_len >0){
						for(j=0;j<=checked_len;j++){
							checked[j] = bck_chk[j];
						}
						free(bck_chk);
					}
					checked[checked_len] = malloc(2*sizeof(int));
					checked[checked_len][0] = act[0];
					checked[checked_len][1] = act[1];
					checked_len++;
				}
			}

			//Dir 3
			act[0] = arr_act[i][0]+1;
			act[1] = arr_act[i][1];
			if(act[0]<maxy){
				if(map[act[0]][act[1]] == 1 && check_in(act,checked,checked_len) == -1){
				
					bck_nxt = arr_nxt;
					arr_nxt = malloc((arr_nxt_len+1)*sizeof(int *));
					if(arr_nxt_len>0){
						for(j=0;	j<=arr_nxt_len;j++){
							arr_nxt[j] = bck_nxt[j];
						}
						free(bck_nxt);
					}
					arr_nxt[arr_nxt_len] = malloc(2*sizeof(int));
					arr_nxt[arr_nxt_len][0] = act[0];
					arr_nxt[arr_nxt_len][1] = act[1];
					arr_nxt_len++;
					temp_arr[act[0]][act[1]] = loop;
					bck_chk = checked;
					checked = malloc((checked_len+1)*sizeof(int *));
					if(checked_len >0){
						for(j=0;j<=checked_len;j++){
							checked[j] = bck_chk[j];
						}
						free(bck_chk);
					}
					checked[checked_len] = malloc(2*sizeof(int));
					checked[checked_len][0] = act[0];
					checked[checked_len][1] = act[1];
					checked_len++;
				}
			}

			//Dir 4
			act[0] = arr_act[i][0]+1;
			act[1] = arr_act[i][1]+1;
			if(act[0]<maxy && act[1]<maxx){
				if(map[act[0]][act[1]] == 1 && check_in(act,checked,checked_len) == -1){
				
					bck_nxt = arr_nxt;
					arr_nxt = malloc((arr_nxt_len+1)*sizeof(int *));
					if(arr_nxt_len>0){
						for(j=0;	j<=arr_nxt_len;j++){
							arr_nxt[j] = bck_nxt[j];
						}
						free(bck_nxt);
					}
					arr_nxt[arr_nxt_len] = malloc(2*sizeof(int));
					arr_nxt[arr_nxt_len][0] = act[0];
					arr_nxt[arr_nxt_len][1] = act[1];
					arr_nxt_len++;
					temp_arr[act[0]][act[1]] = loop;
					bck_chk = checked;
					checked = malloc((checked_len+1)*sizeof(int *));
					if(checked_len >0){
						for(j=0;j<=checked_len;j++){
							checked[j] = bck_chk[j];
						}
						free(bck_chk);
					}
					checked[checked_len] = malloc(2*sizeof(int));
					checked[checked_len][0] = act[0];
					checked[checked_len][1] = act[1];
					checked_len++;
				}
			}

			//Dir 5
			act[0] = arr_act[i][0];
			act[1] = arr_act[i][1]+1;
			if(act[1]<maxx){
				if(map[act[0]][act[1]] == 1 && check_in(act,checked,checked_len) == -1){
				
					bck_nxt = arr_nxt;
					arr_nxt = malloc((arr_nxt_len+1)*sizeof(int *));
					if(arr_nxt_len>0){
						for(j=0;	j<=arr_nxt_len;j++){
							arr_nxt[j] = bck_nxt[j];
						}
						free(bck_nxt);
					}
					arr_nxt[arr_nxt_len] = malloc(2*sizeof(int));
					arr_nxt[arr_nxt_len][0] = act[0];
					arr_nxt[arr_nxt_len][1] = act[1];
					arr_nxt_len++;
					temp_arr[act[0]][act[1]] = loop;
					bck_chk = checked;
					checked = malloc((checked_len+1)*sizeof(int *));
					if(checked_len >0){
						for(j=0;j<=checked_len;j++){
							checked[j] = bck_chk[j];
						}
						free(bck_chk);
					}
					checked[checked_len] = malloc(2*sizeof(int));
					checked[checked_len][0] = act[0];
					checked[checked_len][1] = act[1];
					checked_len++;
				}
			}

			//Dir 6
			act[0] = arr_act[i][0]-1;
			act[1] = arr_act[i][1]+1;
			if(act[0]>0 && act[1]<maxx){
				if(map[act[0]][act[1]] == 1 && check_in(act,checked,checked_len) == -1){
				
					bck_nxt = arr_nxt;
					arr_nxt = malloc((arr_nxt_len+1)*sizeof(int *));
					if(arr_nxt_len>0){
						for(j=0;	j<=arr_nxt_len;j++){
							arr_nxt[j] = bck_nxt[j];
						}
						free(bck_nxt);
					}
					arr_nxt[arr_nxt_len] = malloc(2*sizeof(int));
					arr_nxt[arr_nxt_len][0] = act[0];
					arr_nxt[arr_nxt_len][1] = act[1];
					arr_nxt_len++;
					temp_arr[act[0]][act[1]] = loop;
					bck_chk = checked;
					checked = malloc((checked_len+1)*sizeof(int *));
					if(checked_len >0){
						for(j=0;j<=checked_len;j++){
							checked[j] = bck_chk[j];
						}
						free(bck_chk);
					}
					checked[checked_len] = malloc(2*sizeof(int));
					checked[checked_len][0] = act[0];
					checked[checked_len][1] = act[1];
					checked_len++;
				}
			}

			//Dir 7
			act[0] = arr_act[i][0]-1;
			act[1] = arr_act[i][1];
			if(act[0]>0){
				if(map[act[0]][act[1]] == 1 && check_in(act,checked,checked_len) == -1){
				
					bck_nxt = arr_nxt;
					arr_nxt = malloc((arr_nxt_len+1)*sizeof(int *));
					if(arr_nxt_len>0){
						for(j=0;	j<=arr_nxt_len;j++){
							arr_nxt[j] = bck_nxt[j];
						}
						free(bck_nxt);
					}
					arr_nxt[arr_nxt_len] = malloc(2*sizeof(int));
					arr_nxt[arr_nxt_len][0] = act[0];
					arr_nxt[arr_nxt_len][1] = act[1];
					arr_nxt_len++;
					temp_arr[act[0]][act[1]] = loop;
					bck_chk = checked;
					checked = malloc((checked_len+1)*sizeof(int *));
					if(checked_len >0){
						for(j=0;j<=checked_len;j++){
							checked[j] = bck_chk[j];
						}
						free(bck_chk);
					}
					checked[checked_len] = malloc(2*sizeof(int));
					checked[checked_len][0] = act[0];
					checked[checked_len][1] = act[1];
					checked_len++;
				}
			}

		}
		free(arr_act);
		arr_act = arr_nxt;
		arr_act_len = arr_nxt_len;
		loop++;
	}while(arr_nxt_len >0);

	return temp_arr;	
}

short check_in(int cell[2],int **checked, unsigned int len){
	unsigned int i;
	for(i=0;i<len;i++){
		if(cell[0] == checked[i][0] && cell[1] == checked[i][1]){
			return 0;
		}
	}
	return -1;
}

/*
 * Funciones de comunicación
 */

void snd_log(char *msg){
	fprintf(stderr,"[%s] %s\n",name,msg);
}

size_t read(struct object *root){
	size_t characters;
	size_t bufsize = 0;
	char *buffer = NULL;

	characters = getline(&buffer,&bufsize,stdin);
	__fpurge(stdin);
	if(characters>0){
		read_json(root,buffer,characters);
	}

	return characters;
	
}

void attack(unsigned int ener){
	char *buffer;
	size_t bufsize = 0;
	size_t characters;

	//snprintf(msg,MAX_CANON,"{\"command\": \"attack\",\"energy\": %u}",ener);
	//snd_log(msg);
	//hacer algo
	fprintf(stdout,"{\"command\": \"attack\",\"energy\": %u}\n",ener);
	fflush(stdout);

	//leer respuesta
	characters = getline(&buffer,&bufsize,stdin);
	__fpurge(stdin);
}

void connect(unsigned short x, unsigned short y){
	char *buffer;
	size_t bufsize = 0;
	size_t characters;

	//snprintf(msg,MAX_CANON,"{\"command\": \"connect\",\"destination\": [%u, %u]}",x,y);
	//snd_log(msg);
	//hacer algo
	fprintf(stdout,"{\"command\": \"connect\",\"destination\": [%u, %u]}\n",x,y);
	fflush(stdout);

	//leer respuesta
	characters = getline(&buffer,&bufsize,stdin);
	__fpurge(stdin);
	//snd_log(buffer);
}

void move(short x, short y){
	char *buffer;
	size_t bufsize = 0;
	size_t characters;


	//snprintf(msg,MAX_CANON,"mover x: %hd y: %hd",x,y);
	//snd_log(msg);
	//hacer algo
	fprintf(stdout,"{\"command\": \"move\",\"x\": %hd,\"y\": %hd}\n",x,y);
	fflush(stdout);

	//leer respuesta
	characters = getline(&buffer,&bufsize,stdin);
	__fpurge(stdin);
}

