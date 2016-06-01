library(ggplot2)
library(gridExtra)
library(stringr)
library(Rmisc)


getBase <- function(path)
{
    base = str_extract(path,".*/")
    if (is.na(base)) {
        base = "./"
    }
    return(base);
}

getFileName <- function(path)
{
    base <- getBase(path)
    if (base == "./")
    {
        return(path)
    }
    else
    {
        pathLength <- str_length(path)
        baseLength <- str_length(base)
        return(substr(path,baseLength +1,pathLength))
    }
}

getFilePrefix <- function(path)
{
    file <- getFileName(path)
    return(str_extract(file,"[^.]+"))
}

args <- commandArgs(TRUE)

if (length(args) < 1) {
    cat("Usage: ... <dataFiles>\n");
    quit(status=1)
}

for (i in 1:length(args))
{
    path <- args[i]
    base <- getBase(path)
    filePrefix <- getFilePrefix(path)
    dst <- sprintf("%s%s.png", base, filePrefix)
    data <- read.csv(path)
    plots <- list()
    for (col in c("smse","learning_time","prediction_time")) {
        internalData <- summarySE(data, measurevar=col, groupvars = c("function_name",
                                                                      "solver",
                                                                      "nb_samples"))
        print(internalData)
        g <- ggplot(internalData, aes_string(x="nb_samples", y=col,
                                     ymin=sprintf("%s-se",col),
                                     ymax=sprintf("%s+se",col),
                                     group = "solver", color = "solver"))
        g <- g + facet_wrap(~function_name, nrow = 1, scales = "free")
        g <- g + geom_point(size = 5, shape='x')
        g <- g + geom_line(size = 0.5)
        g <- g + geom_errorbar(color="black", width=0.01)
        g <- g + scale_x_log10(breaks=25 * 2 ** seq(1,16))
        g <- g + scale_y_log10()
        plots <- c(plots,list(g))
    }
    finalG <- arrangeGrob(grobs=plots,nrow=3)
    ggsave(dst, finalG, width=16, height=9)
}
