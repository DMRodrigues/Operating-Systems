# Operating-Systems

Projecto da cadeira de Sistemas Operativos 2014/2015

#####Para mais informações ler: enunciado-ex5-v5.pdf
<br/>
<br/>
Aceita os seguintes comandos:

	-> print
		imprimi o estado actual do buffer, inclusiva o ponteiro do ultimo ficheiro adicionado(head) e do ultimo ficheiro verificado(tail)
		
	 -> sair
		sair do monitor, terminar os processos escritor e leitor e seus correspondentes threads, imprimindo o retorno
		
	-> il
		inverte os locks dos escritores, ou seja, se estavam a usar deixam de usar ou vice-versa

	-> ie
		passa a escrever erradamente, linha sim, linha não, nos ficheiros

<br/>
<br/>
Algumas notas:

Monitor independente que aceita qualquer nome de ficheiro a verificar. Excepto nomes vazios =).

Monitor pode terminar por duas maneiras equivalentes: escrever "sair" ou ^D(Ctrl+D), este ultimo corresponde a um EOT o que indica a conclusão da transmissão.

Para ler um ficheiro como input é necessario ir a linha 148 do monitor.c e repor sleep, pois e necessario que o monitor propage o que le para os filhos (não tive paciência pa corrigir).

Estão dois exemplos de input para o monitor e para o leitor, sao eles respectivamente 'input' e 'input_leitor'.
