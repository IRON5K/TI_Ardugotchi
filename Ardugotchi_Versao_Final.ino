int State[3] = {LOW,LOW,LOW};
int Last_State[3] = {LOW,LOW,LOW};
unsigned long LastDebounceTime[3] = {0,0,0};
const int Delay = 50;

const int Botoes[3] = {2,3,4};
const int Leds[3] = {11,12,13};
unsigned long Intervalos[3];                                 // Comer, Brincar, Dormir
unsigned long Ultimo[5] = {0,0,0,0,0};                       // Comer, Brincar, Dormir, LDR, Saude
int Ler[3];

unsigned int total = 0;
unsigned int media = 0;   
const int numero_de_amostras = 6;
int leituras[numero_de_amostras];
int leitura_index = 0;

int saude = 0;
unsigned long adicionar[3] = {60000,60000,60000};            // Intervalos associados ao 1 minuto que o Jogador demorar a satisfazer as exigencias
bool deduzido[3] = {false,false,false};                      // Comer, Brincar, Dormir
bool morto = false;

void setup(){
  Serial.begin(9600);
  for (int i = 0; i <= 2; i++){
    pinMode(Leds[i], OUTPUT); 
  }
  for (int i = 0; i <= 2; i++) {
    pinMode(Botoes[i],INPUT_PULLUP);
  }
  for (int leitura = 0; leitura < numero_de_amostras; leitura++) {     // Possibilita a mudança do tamanho do Array dependendo do Número de Amostras
    leituras[leitura] = 0;
  }
  randomSeed(analogRead(A1));                                          // A0 já está a ser utilizado pelo LDR  
  Intervalos[0] = random(180000,300001);                               // Gerado um Intervalo entre 3 e 5 minutos para o Intervalo de Comer
  Intervalos[1] = random(120000,240001);                               // Gerado um Intervalo entre 2 e 4 minutos para o Intervalo de Brincar
  Intervalos[2] = random(540000,660001);                               // Gerado um Intervalo entre 9 e 11 minutos para o Intervalo de Dormir
  Serial.println("Ardugotchi Nasceu !");
}

void loop()
{ 
// Ardugotchi Vivo
if (saude <= 25){
  for (int i = 0; i <= 2; i++) {
    Ler[i] = digitalRead(Botoes[i]);                                   // Inicia a leitura dos Botões
  }
  botao(0);                                                            // Botão de Comer
  botao(1);                                                            // Botão de Brincar
  
  // Obter média das últimas medições do LDR
  if ((millis() - Ultimo[3]) >= 60000) {                               // Fazemos a média a cada minuto
    total = total - leituras[leitura_index];
    leituras[leitura_index] = analogRead(A0);
    total = total + leituras[leitura_index];
    leitura_index = leitura_index + 1;
    media = total / numero_de_amostras;
    if (leitura_index >= numero_de_amostras) {
      leitura_index = 0;
    }
    Ultimo[3] = millis();
  }

  // Lógica para Dormir
  if((media) > (1023 * 2/3) || (millis() - Ultimo[2]) >= Intervalos[2]){
    digitalWrite(Leds[2],HIGH);
  }
  // Botão para Dormir
  if ((millis() - LastDebounceTime[2]) > Delay) {                      // Verifica se passou tempo suficiente desde a última leitura do botão
    
    if (State[2] != Ler[2]) {                                          // Verifica se o estado do botão mudou e atualiza o estado dele de tiver mudado   
      State[2] = Ler[2];
    }
    if (Ler[2] != Last_State[2]) {                                     // Reset no tempo do Debounce se houver uma mudança no estado do botão
      LastDebounceTime[2] = millis();
    }
    if (State[2] != Last_State[2] && State[2] == LOW){                 // Verifica se o botão foi pressionado   
      bonificacao(2);                                                  // É chamada a função de Bonificação
      Serial.println("Ardugotchi vai Dormir !");                       // O Ardugotchi vai Dormir
      Ultimo[2] = millis();                                            // Guarda quando o Ardugotchi começa a dormir          
      while ((Ultimo[2] + 300000) > millis()){                         // 300000 ms = 5 minutos (Tempo que o Ardugotchi dorme)
        desligar_leds();
      }
      Serial.println("Ardugotchi Acordou !");
      gerar_intervalo(2);                                              // De Cada vez que o Ardugotchi acorda é gerado um novo intervalo
      for (int i = 0; i <= 2; i++){ 
        Ultimo[i] = millis();                                          // Guarda a última vez que o Ardugotchi Dormiu,Comeu e Brincou
      }     
    }
  Last_State[2] = State[2];                                            // Atualiza o último estado do botão para o próximo loop
  }

  // Lógica da adição de Penalizações ao estado de saúde do Ardugotchi
  for (int i = 0; i <= 2; i++){
    if (millis() - Ultimo[i]  >= Intervalos[i] + adicionar[i]){
      saude += 5;
      adicionar[i] += 60000;
    }
  }
  // Lógica para imprimir o valor do contador de penalizações
  if (millis() - Ultimo[4] >= 60000){
    Ultimo[4] = millis();
    Serial.println(saude);
  } 
}
// Ardugotchi Morto
if ((saude > 25) && (morto == false)){
  morto = true;
  Serial.println("Ardugotchi Morreu !");
  desligar_leds();
}
}

// Função para Desligar os Leds
void desligar_leds(){
  for (int i = 0; i <= 2; i++) {
    digitalWrite(Leds[i], LOW);
  }
}

// Função para Determinar novo Intervalo a cada vez que faz alguma Atividade
void gerar_intervalo(int r){
  if (r == 0) {Intervalos[0] = random(180000,300001);}          // Intervalo do random é aberto
  if (r == 1) {Intervalos[1] = random(120000,240001);}
  if (r == 2) {Intervalos[2] = random(540000,660001);}
}

// Função para Subtrair 5 à Saúde
void bonificacao(int k){
  deduzido[k] = true;
  if ((millis() - Ultimo[k] - Intervalos[k] <= 15000) && (saude != 0) && (deduzido[k] == true)){       // Determina se o Jogador Carregou em até 15 após a notificação
    saude -= 5;
    deduzido[k] = false;    
  }   
}

// Função para Botao de Comer e Brincar
void botao(int j){
  // Lógica para Comer e Brincar
  if((millis() - Ultimo[j]) >= Intervalos[j]){
    digitalWrite(Leds[j],HIGH); 
  }
  // Botão para Comer e Brincar
  if ((millis() - LastDebounceTime[j]) > Delay) {                      // Verifica se passou tempo suficiente desde a última leitura do botão
    
    if (State[j] != Ler[j]) {                                          // Verifica se o estado do botão mudou e atualiza o estado dele de tiver mudado   
      State[j] = Ler[j];
    }
    if (Ler[j] != Last_State[j]) {                                     // Reset no tempo do Debounce se houver uma mudança no estado do botão
      LastDebounceTime[j] = millis();
    }
    if (State[j] != Last_State[j] && State[j] == LOW){                 // Verifica se o botão foi pressionado
      digitalWrite(Leds[j],LOW);                                       // Dá de Comer ao Ardugotchi ou Dá Atenção ao Ardugotchi
      bonificacao(j);                                                  // É chamada a função de Bonificação
      gerar_intervalo(j);                                              // De Cada vez que o Ardugotchi come ou brinca é gerado um novo intervalo
      Ultimo[j] = millis();                                            // Guarda a última vez que o Ardugotchi Comeu ou que teve Atenção  
    }
  Last_State[j] = State[j];                                            // Atualiza o último estado do botão para o próximo loop
  }
}