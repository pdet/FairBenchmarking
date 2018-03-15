library("ggplot2")
library("ggthemes")

textsize <- 26
theme <- theme_few(base_size = textsize) + 
theme(legend.position=c(0, 1))

abplot <- function(fa, fb, la, lb, out) {
	a <- read.table(sprintf("../results/%s", fa))$V1
	b <- read.table(sprintf("../results/%s", fb))$V1

	df <- data.frame(time_sec=c(median(a), median(b)), sys=c("A", "B"))

	pdf(out, height=5, width=6)
	print(ggplot(df, aes(x=sys, y=time_sec, group=sys)) + geom_bar(stat="identity", position = position_dodge(), width=.5) + xlab("") + ylab("Median time (s)") + theme + scale_x_discrete(labels=c(la, lb)))
	dev.off() 
}

abplot("monetdb.default.debug.sf1-q1.tsv", "monetdb.default.optimal.sf1-q1.tsv", "MonetDB (a)", "MonetDB (b)", "compilation-flags.pdf")
abplot("monetdb.default.optimal.sf1-q1.tsv", "hand-sf1-q1.tsv", "MonetDB", "'TimDB'", "hand-rolled.pdf")
abplot("mysql-sf1-q1.tsv", "postgres.optimal.sf1-q1.tsv", "MySQL", "Postgres", "postgres-better-mysql.pdf")
abplot("postgres.default.sf1-q9.tsv", "postgres.optimal.sf1-q9.tsv", "Postgres (a)", "Postgres (b)", "postgres-config.pdf")
abplot("sqlite-sf1-q9-hard-joinorder", "sqlite-sf1-q9-defaullt-joinorder", "SQLite (a)", "SQLite (b)", "sqlite-plan.pdf")

