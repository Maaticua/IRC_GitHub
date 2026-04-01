#ifndef COLORS_HPP
#define COLORS_HPP

// Usage : std::cout << RED << "bonjour" << RESET << std::endl

// Reset
#define RESET  "\033[0m"

// Couleurs de texte
#define BLACK    "\033[30m"
#define RED      "\033[31m"
#define GREEN    "\033[32m"
#define YELLOW   "\033[33m"
#define BLUE     "\033[34m"
#define MAGENTA  "\033[35m"
#define CYAN     "\033[36m"
#define WHITE    "\033[37m"

// Couleurs claires (bright)
#define BBLACK    "\033[90m"
#define BRED      "\033[91m"
#define BGREEN    "\033[92m"
#define BYELLOW   "\033[93m"
#define BBLUE     "\033[94m"
#define BMAGENTA  "\033[95m"
#define BCYAN     "\033[96m"
#define BWHITE    "\033[97m"

// Styles
#define BOLD       "\033[1m"
#define DIM        "\033[2m"
#define UNDERLINE  "\033[4m"
#define BLINK      "\033[5m"
#define REVERSE    "\033[7m"

#endif
