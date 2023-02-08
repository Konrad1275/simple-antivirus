#!/bin/bash

clang-tidy --checks=*,-*braces*,-*readability* Source/main.cpp 

