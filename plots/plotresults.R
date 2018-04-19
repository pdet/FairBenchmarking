library("ggplot2")
library("ggthemes")

textsize <- 28
theme <- theme_few(base_size = textsize, base_family= "serif") + 
theme(legend.position=c(0, 1),  axis.text=element_text(size=textsize, colour = "black"),
        axis.title=element_text(size=textsize, colour = "black"))

abplot <- function(fa, fb, la, lb, out, lims=NULL) {
	a <- read.table(sprintf("../results/%s", fa))$V1
	b <- read.table(sprintf("../results/%s", fb))$V1


	stopifnot(length(a) == 10 && length(b) == 10)

	df <- data.frame(time_sec=c(median(a), median(b)), sys=c("A", "B"), loconf=c(sort(a)[2], sort(b)[2]), hiconf=c(sort(a)[9], sort(b)[9]))

	pdf(out, height=5, width=6)
	print(ggplot(df, aes(x=sys, y=time_sec, group=sys)) + geom_bar(stat="identity", position = position_dodge(), width=.35, fill = "#777777") + xlab("") + ylab("Median time (s)") + theme + scale_x_discrete(labels=c(la, lb)) + geom_errorbar(aes(ymin=loconf, ymax=hiconf), width=.1, size=1.2) + geom_text(aes(label=round(time_sec, 2)), vjust=-.5, hjust=-.5, family="serif", size=7) + scale_y_continuous(limits=lims))

	dev.off() 
}

abplot("monetdb.default.debug.sf1-q1.tsv", "monetdb.default.optimal.sf1-q1.tsv", "MonetDB", "MonetDB*", "compilation-flags.pdf", c(0, 1.75))
abplot("monetdb.default.optimal.sf1-q1.tsv", "hand-sf1-q1.tsv", "MonetDB", "'TimDB'", "hand-rolled.pdf", c(0, 1))
abplot("mysql-sf1-q1.tsv", "postgres.optimal.sf1-q1.tsv", "MariaDB", "Postgres", "postgres-better-mysql.pdf")
abplot("postgres.default.sf1-q9.tsv", "postgres.optimal.sf1-q9.tsv", "Postgres", "Postgres*", "postgres-config.pdf", c(0, 0.6))
abplot("sqlite-sf1-q9-hard-joinorder", "sqlite-sf1-q9-defaullt-joinorder", "SQLite", "SQLite*", "sqlite-plan.pdf")



aggr <- function(f, sys) {
	a <- read.table(sprintf("../results/%s", f))$V1
	data.frame(time_sec=median(a), sys=sys, loconf= sort(a)[2], hiconf=sort(a)[9])
}

	
my <- aggr("mysql-sf1-q1.tsv", "MariaDB")
my$grp <- 1

pg <- aggr("postgres.optimal.sf1-q1.tsv", "Postgres")
pg$grp <- 1

pg2 <- pg
pg2$grp <- 2

sl <- aggr("sqllite.disk.sf1-q1.tsv", "SQLite")
sl$grp <- 2

sl2 <- sl
sl2$grp <- 3

mf <- aggr("mysql-floats-sf1-q1.tsv", "MariaDB*")
mf$grp <- 3

df <- rbind(my, pg, pg2, sl, sl2, mf)
df$grp <- as.factor(df$grp)

pdf("all.pdf", height=5, width=12)
print(ggplot(df, aes(x=sys, y=time_sec, group=sys)) + geom_bar(stat="identity", position = position_dodge(), width=.35, fill = "#777777") + xlab("") + ylab("Median time (s)") + theme  + geom_errorbar(aes(ymin=loconf, ymax=hiconf), width=.1, size=1.2) + geom_text(aes(label=round(time_sec, 2)), vjust=-.5, hjust=-.5, family="serif", size=7) + scale_y_continuous(limits=c(0, 14)) + facet_grid(~grp, scales="free") + theme(strip.text.x = element_blank()))

dev.off() 
