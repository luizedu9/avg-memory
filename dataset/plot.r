#
#Script gerado automaticamente por mem-collector.sh
#
dados <- read.table(file='dados-200ptos-quarto-seg-9.dat');
png('dados-200ptos-quarto-seg-9.png', width=1200);
plot(dados$V1, dados$V2, type="l", main="Uso de Memoria", xlab="Tempo (1 pto/0.25 seg)", ylab="Mem (kb)");
points(dados$V1, dados$V2, col="red", pch=20);
dev.off();
