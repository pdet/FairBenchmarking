library(dplyr)
library(ggplot2)
library(ggthemes)
library(ggrepel)
library(stringr)
library(grid)
library(reshape2)

df3 <- data.frame(x=c('Our System', 'Their System'), y=c('Fast', 'Slow'), z=c(1, 10)) 

theme <- theme_few(base_size = 24) + 
theme(axis.title.y=element_text(vjust=0.9), 
  axis.title.x=element_text(vjust=-0.1),
  axis.ticks.x=element_blank(),
  text=element_text(family="serif"),
  legend.position="none")


pdf("genericbenchmark.pdf", width=7, height=5)

ggplot(df3, aes(x=x, y=z, fill=z)) + geom_bar(stat = "identity", width=0.5) + theme + xlab("") + scale_y_continuous(limits=c(0, 10), breaks=c(1,10), labels=c('Fast', 'Slow')) + ylab("Speed")

dev.off()