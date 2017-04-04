library(ggplot2)

samples <- read.csv("samples.csv");
predictions <- read.csv("predictions.csv");

predictions$nbTrees = as.factor(predictions$nbTrees)

g <- ggplot()

g <- g + geom_point(data=samples, aes(x=input,y=output),
                    #shape='X',
                    size=2)
g <- g + geom_line(data=predictions, aes(x=input,y=output, group=nbTrees, linetype=nbTrees))

g <- g + stat_function(data = data.frame(x=c(-pi,pi)),aes(x),fun=sin, alpha=0.5, size=2)

g <- g + theme_bw()

ggsave("forests_predictions_example.png", width=10,height=5)