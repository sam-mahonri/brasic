# Brasic

Uma implementação C do Tiny Basic, com foco no suporte ao Arduino e outras placas de desenvolvimento, adaptado a linguagem português brasil, com comandos em português e fácil entendimento. É baseado nos códigos escrito por Gordon Brandly na forma de "68000 Tiny Basic" (BASIC Ingles) e depois transportado para C por Michael Field como "Arduino Basic", embora ainda chamado de "Tiny Basic" nos arquivos de origem, e adaptado para Brasic por Sam Mahonri.

O Brasic é uma extensão e modificação do "Tiny Basic" original, adicionando suporte para mais alguns dispositivos, configuráveis no momento da criação, e adaptado para o Brasil pela língua portuguesa. Ele foi projetado para uso no Arduino para o Physalis Palmtop.

Os recursos adicionados incluem suporte para fileio (SD Library), execução automática de um programa a partir do cartão SD, menor área ocupada (PROGMEM), suporte para Entrada / Saída de dados de pinos, suporte para armazenamento EEProm no chip para o seu programa e adaptado para iniciantes em programação com a linguagem toda em português.

Lista completa de instruções e funções suportadas

# Sistema

TCHAU - sai da reinicialização básica e suave no Arduino
FIM - interrompe a execução do programa, também "STOP"
MEM - exibe estatísticas de uso de memória
NOVO - limpa o programa atual
RODAR - executa o programa atual

# Arquivo Cartão IO / SD

ARQUIVOS - lista os arquivos no cartão SD
CARREGAR nomedoarquivo.bas - carrega um arquivo do cartão SD
CORRENTE nomedoarquivo.bas - equivalente a: novo, carrega nomedoarquivo.bas, execute
SALVAR nomedoarquivo.bas - salva o programa atual no cartão SD, sobrescrevendo
EEProm - armazenamento não volátil no chip

EFORMATAR - limpa a memória EEProm
ECARREGAR - carrega o programa no EEProm
ESALVAR - salva o programa atual no EEProm
ELISTAR - imprime o conteúdo do EEProm
ECORRENTE - carrega o programa do EEProm e executa-o

# IO, Documentação

Variável ENTRADA - permite que o usuário insira uma expressão (número ou nome da variável
PEEK (endereço) - obtém um valor na memória (não implementado)
Endereço POKE - defina um valor na memória (não implementado)
Expressão IMPRIMIR - imprima a expressão, também "?"
Material REM - observação / comentário, também "'"

# Expressões, Matemática

A = V, LET A = V - atribui valor a uma variável
+, -, *, / - Matemática
<, <=, =, <>,! =,> =,> - Comparações
ABS (expressão) - retorna o valor absoluto da expressão
RSEED (v) - define a semente aleatória como v
RND (m) - retorna um número aleatório de 0 a m

# Controle

SE expressão declaração - executar declaração se expressão for verdadeira
Variável PARA = início TO fim - início do bloco
Variável PARA = valor do início ao fim do passo - inicie o bloco com o passo
PROXIMO - fim do bloco
Número de roupa IRPARA - continue a execução neste número de linha
Número de roupa GOSUB - ligue para uma sub-rotina neste número de linha
RETORNAR - retornar de uma sub-rotina





# Pinos IO

ESPERE * - espera (em milissegundos) *
Pino ESCREVERD, valor - defina o pino com um valor (ALTO, AL, BAIXO, BA)
Pino ESCREVERA, valor - ajuste o pino com valor analógico (PWM) 0..255
LERD (pino) - obtém o valor do pino
LERA (analogPin) - obtém o valor do pino analógico

# Exemplos

Entrada do usuário

Permita que um usuário insira um novo valor para uma variável, insira um número como '33' ou '42' ou uma variável como 'b'.

> 10 A = 0
> 15 B = 999
> 20 IMPRIMIR "A é", A
> 30 IMPRIMIR "Digite um novo valor";
> 40 ENTRADA A
> 50 IMPRIMIR "A é agora", A

Piscar

Conecte um LED entre o pino D3 e o terra (GND)

> 10 PARA A = 0 A 10
> 20 ESCREVERD 3, ALTO
> 30 ESPERE 250
> 40 ESCREVERD 3, BAIXO
> 50 ESPERE 250
> 60 PROXIMO A


Apagar e ligar suavemente, efeito tipo “Fade” (Apenas Portas PWM)

Conecte um LED entre o pino 3 e o terra (GND)

> 10 PARA A = 0 A 10
> 20 PARA B = 0 A 255
> 30 ESCREVERA 3, B
> 40 ESPERE 10
> 50 PROXIMO B
> 60 PARA B = 255 A 0 PASSO -1
> 70 ESCREVERA 3, B
> 80 ESPERE 10
> 90 PROXIMO B
> 100 PROXIMO A

Botão + LED

Conecte um potenciômetro entre o analógico 2 e o terra, conduzido do digital 3 e o terra. Se o botão estiver em 0, ele para.

> 10 A = LERA (2)
> 20 IMPRIMIR A
> 30 B = A / 4
> 40 ESCREVERA 3, B
> 50 SE A == 0 GOTO 100
> 60 IRPARA 10
> 100 IMPRIMIR "Concluído".

Exemplo ECORRENTE

Escreva um pequeno programa, armazene-o na EEPROM. Mostraremos que as variáveis não são apagadas quando o encadeamento acontece.

> EFORMATAR
> 10 A = A + 2
> 20 IMPRIMIR A
> 30 IMPRIMIR "Da eeprom!"
> 40 SE A = 12 GOTO 100
> 50 IMPRIMIR "Não deveria estar aqui."
> 60 FIM
> 100 IMPRIMIR "Olá!"
Em seguida, armazene-o no EEProm

> ESALVAR

Agora, crie um novo programa na memória principal e execute-o

> NOVO
> 10 A = 10
> 20 IMPREMIR A
> 30 IMPRIMIR "A partir da RAM!"
> 40 ECORRENTE

Listar ambos e executar

> ELISTAR
> LISTAR
> RODAR

# Suporte a dispositivos

Arduino - ATMega 168 (~ 100 bytes disponível)
Arduino - ATMega 368 (~ 1100 bytes disponíveis)

Cartões SD (via SD Library, para comandos FILES, CARREGAR, SAVE, usa 9k de ROM)

EEProm (via EEProm Library, usa 500 bytes de ROM)
E / S serial - console de comando

# Peculiaridades e limitações conhecidas

- Se CARREGAR ou SALVAR forem chamados, ARQUIVOS falhará nas listagens subsequentes.
- Os cartões SD não podem ser trocados durante a execução do Brasic. É necessária uma redefinição (TCHAU ou Sair e entrar no Terminal) entre trocas.
- Está limitado apenas a interação via Serial, mas no futuro poderá ser usado com teclado externo e displays.



# Referências

- Brasic - Sam Mahonri – Adaptação para o português brasileiro e uso de iniciantes em programação. - sam-mahonri no Github.

- Tiny Basic 68k - Página do Projeto Gordon Brandly (via archive.org).

- Arduino Basic / Tiny Basic C - Michael Field Project Page.

- Tiny Basic Plus - Scott Lawrence yorgle@gmail.com Página do Github.

- Jurg Wullschleger - Correção para operações unárias e espaço em branco nas expressões.

- BleuLlama – exemplos dos códigos do texto – Página no Github.

Licença do Tiny Basic C por Michael Field:

Michael Field disponibilizou uma licença semelhante a MIT License.

Licença* do Brasic por Sam Mahonri:

Sam Mahonri disponibiliza o Brasic em uma licença GPL Versão 3.

*Um programa com licença MIT pode ser incorporado em um programa com licença GPL que é o caso do Brasic.

GNU GPL General Public License: 
    1. A liberdade de executar o programa, para qualquer propósito Nº 0 
    2. A liberdade de estudar como o programa funciona e adaptá-lo às suas necessidades Nº 1. O acesso ao código-fonte é um pré-requisito para esta liberdade. 
    3. A liberdade de redistribuir cópias de modo que você possa ajudar ao seu próximo Nº 2. 
    4. A liberdade de aperfeiçoar o programa e liberar os seus aperfeiçoamentos, de modo que toda a comunidade beneficie deles Nº 3. O acesso ao código-fonte é um pré-requisito para esta liberdade.


