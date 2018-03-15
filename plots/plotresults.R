library("ggplot2")
library("ggthemes")

textsize <- 28
theme <- theme_few(base_size = textsize, base_family= "serif") + 
theme(legend.position=c(0, 1),  axis.text=element_text(size=textsize, colour = "black"),
        axis.title=element_text(size=textsize, colour = "black"))

abplot <- function(fa, fb, la, lb, out) {
	a <- read.table(sprintf("../results/%s", fa))$V1
	b <- read.table(sprintf("../results/%s", fb))$V1


	stopifnot(length(a) == 10 && length(b) == 10)

	df <- data.frame(time_sec=c(median(a), median(b)), sys=c("A", "B"), hiconf=c(sort(a)[2], sort(b)[2]), loconf=c(sort(a)[9], sort(b)[9]))

	pdf(out, height=5, width=6)
	print(ggplot(df, aes(x=sys, y=time_sec, group=sys)) + geom_bar(stat="identity", position = position_dodge(), width=.4, fill = "#777777") + xlab("") + ylab("Median time (s)") + theme + scale_x_discrete(labels=c(la, lb)) + geom_errorbar(aes(ymin=loconf, ymax=hiconf), width=.1, size=1.2))

	dev.off() 
}

abplot("monetdb.default.debug.sf1-q1.tsv", "monetdb.default.optimal.sf1-q1.tsv", "Monet (a)", "Monet (b)", "compilation-flags.pdf")
abplot("hyper-sf1-q1.tsv", "hand-sf1-q1.tsv", "HyPer", "'TimDB'", "hand-rolled.pdf")
abplot("mysql-sf1-q1.tsv", "postgres.optimal.sf1-q1.tsv", "MariaDB", "Postgres", "postgres-better-mysql.pdf")
abplot("postgres.default.sf1-q9.tsv", "postgres.optimal.sf1-q9.tsv", "Postgres (a)", "Postgres (b)", "postgres-config.pdf")
abplot("sqlite-sf1-q9-hard-joinorder", "sqlite-sf1-q9-defaullt-joinorder", "SQLite (a)", "SQLite (b)", "sqlite-plan.pdf")

