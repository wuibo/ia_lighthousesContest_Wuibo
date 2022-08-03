#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <utils.h>
#include <math.h>


int main(){
	int MAX_CANON = 200;
	char msg[MAX_CANON];
	char *buffer;
	size_t bufsize = 0;
	size_t characters;
	char *i_point, *f_point;
	char *temp;
	int temp_size;
	struct game juego;
	struct player player;
	struct objetivo obj;
	//nuevo
	struct object root,otemp;
	struct object *p_temp;
	int id;
	int player_num;
	short mov[8][2] = {{-1,-1},{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0}};
	unsigned short i,f_turn;
	unsigned int t_ener,ts_ener,n_ener,ns_ener,t_move,n_move;
	unsigned int t_ener0,t_ener1,t_ener2,t_ener3,t_ener4,t_ener5,t_ener6,t_ener7;
	unsigned short c_view,ymov_view,xmov_view;
	unsigned int t_dist,n_dist;

	//fprintf(stderr,"HOla mundo!\n");
	//snprintf(msg,MAX_CANON,"Hola mundo");
	//snd_log(msg);

	f_turn = 0;
	characters = read(&root);


	//número de jugador	
	id = json_get_item_by_name(root,"player_num");
	if(id<0){
		snd_log("no se encuentra player_num");
	}else{
		p_temp = root.content;
		otemp = p_temp[id];
		player_num = (int) json_get_number(otemp);
	}

	//inicializar jugador
	player.energia = 0;
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
			player.posicion[1] = (short) json_get_number(otemp);
			otemp = p_temp[1];
			player.posicion[0] = (short) json_get_number(otemp);
		}
	}

	//cargar el juego
	parse_game(&juego,root);
	//snprintf(msg,MAX_CANON,"faros: %hu tri: %hu", juego.faros_count, juego.tri_count);
	//snd_log(msg);
/*
	//mostrar triangulos
	for(i=0;i<juego.tri_count;i++){
		snprintf(msg,MAX_CANON,"traingulo %u, faro0: %u, faro1: %u, faro2: %u",i,juego.triangulos[i].faros[0],juego.triangulos[i].faros[1],juego.triangulos[i].faros[2]);
		snd_log(msg);	
	}
*/
	fprintf(stdout,"{\"name\": \"WuiboEE30\"}\n");
	fflush(stdout);
	characters = 0;
	while(characters<=bufsize){
		characters = getline(&buffer,&bufsize,stdin);
		__fpurge(stdin);
		//snprintf(msg,MAX_CANON,"chars: %zu size: %zu",characters,bufsize);
		//snd_log(msg);
		read_json(&root,buffer,characters);
		parse_turn(&player,&juego,root);

		//objetivo
		obj = get_objetive(player,juego);
		//snprintf(msg,MAX_CANON,"objetivo next: %hu connect:%hu energy: %hu, tri: %hu",obj.next,obj.connect,obj.energy,obj.tri);
		//snd_log(msg);
		//comprobar energia
		if(player.energia < obj.n_energy){
			c_view = player.v_maxx/2;
			t_ener = 0;
			t_move = 0;
			ts_ener = 0;
			//movimiento por energia
			//snd_log("movimiento energia");
			for(i=0;i<8;i++){
				ymov_view = c_view+mov[i][0];
				xmov_view = c_view+mov[i][1];
				t_ener0 = player.view[ymov_view-1][xmov_view-1];
				t_ener1 = player.view[ymov_view][xmov_view-1];
				t_ener2 = player.view[ymov_view+1][xmov_view-1];
				t_ener3 = player.view[ymov_view+1][xmov_view];
				t_ener4 = player.view[ymov_view+1][xmov_view+1];
				t_ener5 = player.view[ymov_view][xmov_view+1];
				t_ener6 = player.view[ymov_view-1][xmov_view+1];
				t_ener7 = player.view[ymov_view-1][xmov_view];
				n_ener = player.view[ymov_view][xmov_view];
				ns_ener = fmax(t_ener0,fmax(t_ener1,fmax(t_ener2,fmax(t_ener3,
					fmax(t_ener4,fmax(t_ener5,fmax(t_ener6,t_ener7)))))));
				if(n_ener > t_ener){
					//mejor energia, cambiar
					t_ener = n_ener;
					ts_ener = ns_ener;
					t_move = i;
				}else if(n_ener == t_ener){
					//igual, comparar segunda
					if(ns_ener > ts_ener){
						//mejor cambiar
						t_ener = n_ener;
						ts_ener = ns_ener;
						t_move = i;
					}else if(ns_ener == ts_ener){
						//igual mirar el que acerca al faro
						t_dist = juego.faros[obj.next].distancia[player.posicion[0]+mov[t_move][0]][player.posicion[1]+mov[t_move][1]];
						n_dist = juego.faros[obj.next].distancia[player.posicion[0]+mov[i][0]][player.posicion[1]+mov[i][1]];
						if(n_dist<t_dist){
							t_ener = n_ener;
							ts_ener = ns_ener;
							t_move = i;
						}
					}
				}
			}
			f_turn = 0;
			move(mov[t_move][1],mov[t_move][0]);
		}else{
			//movimiento al objetivo
			//nprintf(msg,MAX_CANON,"mover al objetivo px: %hu, py: %hu, fx: %hu, fy: %hu",player.posicion[1],player.posicion[0],juego.faros[obj.next].posicion[1],juego.faros[obj.next].posicion[0]);
			//snd_log(msg);
			if(player.posicion[0] == juego.faros[obj.next].posicion[0] && player.posicion[1] == juego.faros[obj.next].posicion[1]){
				//ya estamos en el objetivo
				if(juego.faros[obj.next].dueno==juego.player_num){
					//ya es nuestro, mirar energ
					if(obj.energy>0 && f_turn ==0){
						//atacar
						//snd_log("atacar");
						f_turn = 1;
						attack(obj.energy+((player.energia-obj.energy)/2));
					}else{
						//no es necesario atacar, conectar
						//snd_log("conectar");
						f_turn =1;
						connect(juego.faros[obj.connect].posicion[1],juego.faros[obj.connect].posicion[0]);
					}
				}else{
					//no es nuestro, atacar
					//snd_log("atacar");
					f_turn = 1;
					attack(obj.energy+(player.energia-obj.energy)/2);
				}
			}else{
				//no estamos, a moverse
				f_turn = 0;
				t_dist = juego.maxx*juego.maxy;
				t_ener = 0;
				c_view = player.v_maxx/2;
				t_move = 0;
				for(i=0;i<8;i++){
					n_dist = juego.faros[obj.next].distancia[player.posicion[0]+mov[i][0]][player.posicion[1]+mov[i][1]];
					if(n_dist < t_dist){
						t_dist=n_dist;
						t_ener=player.view[c_view+mov[i][0]][c_view+mov[i][1]];
						t_move = i;
					}else if(n_dist == t_dist){
						//iguales mirar energia
						n_ener=player.view[c_view+mov[i][0]][c_view+mov[i][1]];
						if(n_ener > t_ener){
							t_dist = n_dist;
							t_ener = n_ener;
							t_move = i;
						}
					}
				}
				move(mov[t_move][1],mov[t_move][0]);
			}
		}

	}

	return 0;

}

