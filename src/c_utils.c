#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <utils.h>

/*
 * CONSTANTES
 */
#define ST_NL 0 //Esperamos [
#define ST_NM 1 //esperamos numero, coma o ]
#define ST_IL 2 //fin de la linea


/*
 * funciones utiles
 */



short parse_game(struct game *juego, char *json){
	char *i_point, *f_point;
	char *temp;
	int temp_size;
	unsigned short state = ST_NL;
	unsigned short line = 0;
	char * temp_num;
	char * back_num;
	unsigned short *temp_line;
	unsigned short *back_line;
	unsigned short **temp_map;
	unsigned short **back_map;
	char array_char;
	unsigned short maxy=0;
	unsigned short maxx;
	unsigned short light_count = 0;
	unsigned short act_num=0,act_x=0;
	struct faro *temp_faros;
	struct faro *back_faros;
	struct faro t_faro;
	int i,j;

	//numero de jugador
	if((i_point = strstr(json,"player_num")) == NULL){
		fprintf(stderr, "No player num\n");
		return -1;
	}
	if((f_point = strstr(i_point,",")) == NULL){
		if((f_point = strstr(i_point,"}")) == NULL){
			fprintf(stderr, "E1: json erroneo\n");
		}
		return -1;
	}
	temp_size = f_point-i_point-12;
	temp = malloc(temp_size * sizeof(char)+1);
	strncpy(temp,i_point+12,temp_size);
	juego->player_num = atoi(temp);
	fprintf(stderr,"playernum: %d\n",juego->player_num);
	free(temp);

	//cantidad de jugadores
	if((i_point = strstr(json,"player_count")) == NULL){
		fprintf(stderr, "No player count\n");
		return -1;
	}
	if((f_point = strstr(i_point,",")) == NULL){
		if((f_point = strstr(i_point,"}")) === NULL){
			fprintf(stderr, "E2: json erroneo");
		}
		return -1;
	}
	temp_size = f_point-i_point-14;
	temp = malloc(temp_size * sizeof(char)+1);
	strncpy(temp,i_point+14,temp_size);
	juego->player_count = atoi(temp);
	free(temp);
	fprintf(stderr,"playercount: %d\n",juego->player_count);

	//posicion
	if((i_point = strstr(json,"position")) == NULL){
		fprintf(stderr, "No position\n");
	}
	if((f_point = strstr(i_point,"],")) == NULL){
		if((f_point = strstr(i_point,"]}")) == NULL){
			if((f_point = strstr(i_point,"]\n,")) == NULL){
				if((f_point = strstar(i_point,"]\n}")) == NULL){
					fprintf(stderr, "json erroneo");
				}
			}
		}
	}
	temp_size = f_point-i_point

	//mapa
	if((i_point = strstr(json,"map")) == NULL){
		fprintf(stderr,"No map\n");
		return -1;
	}
	if((f_point = strstr(i_point,"]]")) == NULL){
		fprintf(stderr, "E3 json erroneo\n");
		return -1;
	}
	if((i_point = strstr(i_point,"[")) == NULL){
		fprintf(stderr,"E4 json erroneo\n");
		return -1;
	}
	temp_size = f_point-i_point+3;
	temp = malloc(temp_size * sizeof(char)+1);
	strncpy(temp,i_point,temp_size);
	fprintf(stderr,"Empezamos mapa\n");
	//empezamso en 1 para saltar el primer [
	for(i=1;i<temp_size;i++){
		array_char = temp[i];
		switch(state){
			case ST_NL:
				//se espera [ o espacio entre líneas en caso contrario error
				switch(array_char){
					case '[':
						state = ST_NM;
						break;
					case ' ':
						break;
					default:
						fprintf(stderr,"error parseando mapa: [ esperado\n");
						return -1;
						break;
				}
				break;
			case ST_NM:
				//se espera numero, coma, espacio o ]
				switch(array_char){
					case ',':
						//cambio de número
						back_line = temp_line;
						temp_line = malloc((act_x+1)*sizeof(unsigned short));
						for(j=0;j<act_x;j++){
							temp_line[j] = back_line[j];
						}
						temp_line[act_x] = atoi(temp_num);
						act_x++;
						if(act_x > 1){
							free(back_line);
						}
						free(temp_num);
						act_num=0;
						break;
					case ' ':
						//ignorar
						break;
					case ']':
						//fin de linea (primero fin de numero)
						back_line = temp_line;
						temp_line = malloc((act_x+1)*sizeof(unsigned short));
						for(j=0;j<act_x;j++){
							temp_line[j] = back_line[j];
						}
						temp_line[act_x] = atoi(temp_num);
						act_x++;
						free(back_line);
						free(temp_num);
						act_num=0;
						//fin de linea
						//comprobar que todas las lineas son iguales
						if(maxy>0){
							if(act_x!=maxx){
								fprintf(stderr,"todas las filas del mapa deben tener la misma longitud");
								return -1;
							}
						}else{
							maxx = act_x;
						}
						act_x = 0;
						back_map = temp_map;
						temp_map = malloc((maxy+1)*sizeof(unsigned short*));
						for(j=0;j<maxy;j++){
							temp_map[j] = back_map[j];
						}
						temp_map[maxy] = temp_line;
						maxy++;
						if(maxy>1){
							free(back_map);
						}
						state = ST_IL;
						break;
					default:
						if(array_char<'0'||array_char>'9'){
							fprintf(stderr,"error parseando mapa, caracter desconocido");
							return -1;
						}
						back_num = temp_num;
						temp_num = malloc((act_num+1)*sizeof(char *));
						for(j=0;j<act_num;j++){
							temp_num[j] = back_num[j];
						}
						temp_num[act_num] = array_char;
						act_num++;
						
						if(act_num > 1){
							free(back_num);
						}
				}
				break;
			case ST_IL:
				//se espera , y si ] fin del array
				switch(array_char){
					case ',':
						state = ST_NL;
						break;
					case ']':
						fprintf(stderr,"fin mapa\n");
						juego->maxy = maxy;
						juego->maxx = maxx;
						juego->map = temp_map;
						break;
				}
				break;
			default:
				fprintf(stderr,"Error parseando el mapa\n");
				return -1;
		}
	}

	//imprimir mapa
	
	for(j=maxy-1;j>=0;j--){
		for(act_x=0;act_x<maxx;act_x++){
			if(juego->map[j][act_x]==1){
				fprintf(stderr," ");
			}else{
				fprintf(stderr,"#");
			}
		}
		fprintf(stderr,"\n");
	}
	

	//cargar faros
	if((i_point = strstr(json,"lighthouses")) == NULL){
		fprintf(stderr,"No lighthouses\n");
		return -1;
	}
	if((f_point = strstr(i_point,"]]")) == NULL){
		fprintf(stderr,"json erroneo\n");
		return -1;
	}
	if((i_point = strstr(i_point,"[")) == NULL){
		fprintf(stderr, "json erroneo\n");
		return -1;
	}
	temp_size = f_point-i_point+3;
	temp = malloc(temp_size * sizeof(char)+1);
	strncpy(temp,i_point,temp_size);
	state = ST_NL;
	act_num = 0;
	act_x = 0;
	fprintf(stderr,"comienza faros\n");
	//empezamos en 1 para saltar el [ inicial
	for(i=1;i<temp_size;i++){
		array_char = temp[i];
		switch(state){
			case ST_NL:
				//se esperoa [ o espacio
				switch(array_char){
					case '[':
						state = ST_NM;
						break;
					case ' ':
						break;
					default:
						fprintf(stderr,"error parseando faros (%c): [ esperado\n",array_char);
						return -1;
						break;
				}
				break;
			case ST_NM:
				//se espera numero, coma o ]
				switch(array_char){
					case ',':
						//cambio de número
						back_line = temp_line;
						temp_line = malloc((act_x+1)*sizeof(unsigned short));
						for(j=0;j<act_x;j++){
							temp_line[j] = back_line[j];
						}
						temp_line[act_x] = atoi(temp_num);
						act_x++;
						if(act_x > 1){
							free(back_line);
						}
						free(temp_num);
						act_num=0;
						break;
					case ' ':
						//ignorar
						break;
					case ']':
						//fin del faro (primero fin de numero)
						back_line = temp_line;
						temp_line = malloc((act_x+1)*sizeof(unsigned short));
						for(j=0;j<act_x;j++){
							temp_line[j]=back_line[j];
						}
						temp_line[act_x] = atoi(temp_num);
						act_x++;
						free(back_line);
						free(temp_num);
						act_num=0;
						//fin de linea
						//comprobar que son 2 numeros
						if(act_x != 2){
							fprintf(stderr,"los faros deben tener 2 coordenadas\n");
							return -1;
						}
						act_x = 0;
						back_faros = temp_faros;
						temp_faros = malloc((light_count+1)*sizeof(struct faro));
						for(j=0;j<light_count;j++){
							temp_faros[j] = back_faros[j];
						}
						t_faro.posicion = temp_line;
						temp_faros[light_count] = t_faro;
						light_count++;
						if(light_count>1){
							free(back_faros);
						}
						state = ST_IL;
						break;					
					default:
						if(array_char<'0'||array_char>'9'){
							fprintf(stderr,"error parseando faros, caracter desconocido");
							return -1;
						}
						back_num = temp_num;
						temp_num = malloc((act_num+1)*sizeof(char *));
						for(j=0;j<act_num;j++){
							temp_num[j] = back_num[j];
						}
						temp_num[act_num] = array_char;
						act_num++;
						if(act_num >1){
							free(back_num);
						}
						break;
				}
				break;
			case ST_IL:
				//se espera , y si ] fin de array
				switch(array_char){
					case ',':
						state = ST_NL;
						break;
					case ']':
						juego->faros = temp_faros;
						juego->faros_count = light_count;
						break;
				}
				break;
			default:
				fprintf(stderr,"Error parseando faros\n");
				return -1;
		}
	}

	//imprimier faros
	fprintf(stderr,"imprimir faros: %d\n",juego->faros_count);
	for(j=0;j<juego->faros_count;j++){
		fprintf(stderr,"[%d,%d]\n",juego->faros[j].posicion[0],juego->faros[j].posicion[1]);
	}
	

}
