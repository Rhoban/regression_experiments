library(ggplot2)
library(gridExtra)
library(plyr)
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

# Each arg starting by -- specifies the beginning of a category:
# Ex: --cat1 path1 path2 --cat2 path3 path4 path5 ...
# Results is a named list with such as:
# result[["cat1"]] = c(path1, path2)
# result[["cat2"]] = c(path3, path4, path5)
argsToCategories <- function(args)
{
    result <- list()
    category <- "Unknown"
    for (arg in args) {
        if (str_length(arg) > 2 && str_sub(arg,1,2) == "--") {
            category = str_sub(arg,3)
            result[[category]] = c()
        }
        else {
            result[[category]] = c(result[[category]], arg)
        }
    }
    return(result)
}

compareMax <- function (data, dst)
{
    plots <- list()
    for (col in c("squared_loss","squared_error","compute_max_time")) {
        internalData <- data
        internalData$y = data[,col]
        groupVar <- c("function_name", "group", "nb_samples")
        #internalData <- summarySE(data, measurevar=col, groupvars = c("function_name",
        #                                                              "group",
        #                                                              "nb_samples"))
        #internalData$ymin = internalData[,col] - internalData$ci
        #internalData$ymax = internalData[,col] + internalData$ci
        internalData <- ddply(internalData, groupVar, summarise,
                              mean = mean(y),
                              median = quantile(y, probs=c(0.5)),
                              ymin = quantile(y, probs=c(0.45)),
                              ymax = quantile(y, probs=c(0.55)))
        print(internalData)
        g <- ggplot(internalData, aes_string(x = "nb_samples", y = "mean",
                                             ymin = "ymin", ymax = "ymax",
                                             group = "group", color = "group", fill = "group"))
        g <- g + facet_wrap(~function_name, nrow = 1, scales = "free")
        g <- g + geom_ribbon(size = 0.1, alpha=0.1)
        g <- g + geom_point(size = 1, shape='x')
        g <- g + geom_line(size = 0.5)
        g <- g + scale_x_log10(breaks=10 * (2 ** seq(0,20)))
        g <- g + scale_y_log10()
        g <- g + theme(axis.text.x = element_text(angle = 90, hjust=1, vjust=0.5))
        g <- g + ylab(col)
        plots <- c(plots,list(g))
    }
    finalG <- arrangeGrob(grobs=plots,nrow=3)
    ggsave(dst, finalG, width=16, height=9)    
}

compareSMSE <- function (data, dst)
{
    plots <- list()
    for (col in c("smse","learning_time","prediction_time")) {
        internalData <- data
        internalData$y = data[,col]
        groupVar <- c("function_name", "group", "nb_samples")
        #internalData <- summarySE(data, measurevar=col, groupvars = c("function_name",
        #                                                              "group",
        #                                                              "nb_samples"))
        #internalData$ymin = internalData[,col] - internalData$ci
        #internalData$ymax = internalData[,col] + internalData$ci
        internalData <- ddply(internalData, groupVar, summarise,
                              mean = mean(y),
                              median = quantile(y, probs=c(0.5)),
                              ymin = quantile(y, probs=c(0.4)),
                              ymax = quantile(y, probs=c(0.6)))
        print(internalData)
        g <- ggplot(internalData, aes_string(x = "nb_samples", y = "mean",
                                             ymin = "ymin", ymax = "ymax",
                                             group = "group", color = "group", fill = "group"))
        g <- g + facet_wrap(~function_name, nrow = 1, scales = "free")
        g <- g + geom_ribbon(size = 0.1, alpha=0.1)
        g <- g + geom_point(size = 1, shape='x')
        g <- g + geom_line(size = 0.5)
        g <- g + scale_x_log10(breaks=10 * 2 ** seq(0,20))
        g <- g + scale_y_log10()
        g <- g + theme(axis.text.x = element_text(angle = 90, hjust=1, vjust=0.5))
        g <- g + ylab(col)
        plots <- c(plots,list(g))
    }
    finalG <- arrangeGrob(grobs=plots,nrow=3)
    # If meaningfull, produce a 16:9 image, otherwise produce a larger image
    plotHeight <- 9
    plotWidth <- 16
    nbFunctions <-length(unique(data$function_name))
    if (nbFunctions > 3) { plotWidth <- plotHeight * nbFunctions * 16 / 9 / 3 }
    ggsave(dst, finalG, width=plotWidth, height=plotHeight)
}

compareByCategories <- function (categories)
{
    # STEP 1: gathering data
    data <- NULL
    for (catName in names(categories))
    {
        for (path in categories[[catName]])
        {
            pathData <- read.csv(path)
            pathData$category = catName
            if (is.null(data)) {
                data <- pathData
            }
            else {
                data <- rbind(data, pathData)
            }
        }
    }
    #STEP 2: create groups:
    if (length(categories) > 1) {
        data$group = sprintf("%s:%s", data$category, data$method)
    }
    else {
        data$group = data$method;
    }
    #STEP 3: plot
    if ("smse" %in% names(data)) {
       compareSMSE(data,"smse.png")
    }
    if ("squared_loss" %in% names(data)) {
       compareMax(data, "max.png")
    }
}

args <- commandArgs(TRUE)

if (length(args) < 1) {
    cat("Usage: ... <dataFiles>\n");
    quit(status=1)
}

categories <- argsToCategories(args)

compareByCategories(categories)
