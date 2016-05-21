library(ggplot2)
library(gridExtra)
library(stringr)

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
    data <- aggregate(. ~ function_name+ solver + nb_samples, data, mean)
    print(data)
    plots <- list()
    for (col in c("smse","learning_time","prediction_time")) {
        g <- ggplot(data, aes_string(x="nb_samples", y=col, group = "solver", color = "solver"))
        g <- g + facet_wrap(~function_name, nrow = 1, scales = "free")
        g <- g + geom_point(size = 0.5)
        g <- g + geom_line(size = 0.2)
        g <- g + scale_y_log10()
        plots <- c(plots,list(g))
    }
    finalG <- arrangeGrob(grobs=plots,nrow=3)
    ggsave(dst, finalG, width=16, height=9)
}
