# threading-server

Um sistema de servidor deve sempre disponível para pode responder a requisições feitas por outros
processos ou threads, podendo ser no mesmo computador ou em outros computadores. Uma
abordagem útil para lidar com múltiplas requisições é utilizar Pool de Threads. Esa abordagem permite
que seja diminuído a carga de trabalho em criar novas threads toda a vez que necessita atender uma
requisição e torna o sistema mais “leve” comparado a uso de múltiplos processos.

Com isso, imagine que você está criando uma espécie de servidor local que funciona com pool de
threads e que cada thread (worker) se comunica com processos que solicitam alguma informação. O
processo “servidor” que possui os workers deve permitir enviar e receber solicitações via pipe. Os
processos que gostaria de obter os serviços do servidor devem se conectar ao pipe correto. As threads
da pool de threads podem ter serviços diferentes para simular um servidor como:

- Algumas threads respondem a solicitações de números e caso algum cliente queira obter isso
deve se conectar ao pipe desse tipo de solicitação.
- Algumas threads respondem a solicitação de string (uma só) e caso algum cliente queira obter
isso, deve se conectar ao pipe desse tipo de solicitação.

A quantidade de threads na pool é um critério que fica a sua escolha, mas lembre-se sobre dimensionar
a quantidade diante do hardware que você tem disponível.

Na figura, dois processos clientes estão conectados ao processo servidor via pipe, porém que lida com
os pipes são as threads na pool. As threads TN são as responsáveis por responder requisições de
números, enquanto que as threads TS são as responsáveis por responder as requisições de string. 