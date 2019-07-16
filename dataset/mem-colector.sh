#
# Coleta informacoes de memoria
#

# Verifica se os parametros informados estao corretos
# Se nao informaram parametros
if [ $# -ne 4 ];
then
	echo
	echo "Erro de sintaxe. Usar: mem-collect.sh <repeticoes> <num obs> <intervalo> <arq saida>"
	echo 
	echo "Exemplo: ./mem-collect.sh 3 10 1 dados.txt"
	echo
	exit -1;
fi

# Obtem parametros informados
REPETICOES=$1
NUMPTOS=$2
INTERVALO=$3
ARQSAIDA=$4

# Cria contador de repeticoes
REPCOUNT=0

# Repete a coleta de acordo com o especificado
until [ $REPCOUNT -ge $REPETICOES ];
do
	# Cria o arquivo de saida
	echo -n "" > $ARQSAIDA-$REPCOUNT".dat"

	# Executa a aplicacao a quantidade de repeticoes especificada
	CONTADOR=0
	until [ $CONTADOR -ge $NUMPTOS ];
	do
		echo "Coletando obs $CONTADOR..."

		# Filtra as linhas de interesse
		TOTAL=$(cat /proc/meminfo | grep "MemTotal:" | awk '{print $2}')
		LIVRE=$(cat /proc/meminfo | grep "MemFree:"  | awk '{print $2}')

		# Calcula a memoria usada pela diferenca da total - livre
		let USADO=TOTAL-LIVRE

		# Salva tabela em arquivo de saida
		echo "$CONTADOR  $USADO" >> $ARQSAIDA-$REPCOUNT".dat"

		# Aguarda o intervalo entre uma coleta e outra
		sleep $INTERVALO

		# Incrementa contador
		let CONTADOR=CONTADOR+1
	done

	# Cria script R automaticamente para plotar o resultado
	# da coleta
	echo '#' > plot.r
	echo "#Script gerado automaticamente por mem-collector.sh" >> plot.r
	echo '#' >> plot.r
	echo "dados <- read.table(file='$ARQSAIDA-$REPCOUNT.dat');" >> plot.r
	echo "png('$ARQSAIDA-$REPCOUNT.png', width=1200);" >> plot.r
	echo 'plot(dados$V1, dados$V2, type="l", main="Uso de Memoria", xlab="Tempo (1 pto/'$INTERVALO' seg)", ylab="Mem (kb)");' >> plot.r
	echo 'points(dados$V1, dados$V2, col="red", pch=20);' >> plot.r
	echo "dev.off();" >> plot.r

	# Executa R script do terminal
	Rscript plot.r > /dev/null

	# Incrementa contador
	let REPCOUNT=REPCOUNT+1
done

