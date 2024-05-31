#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <fstream>

namespace file {
    /**
     * @class FileHandler
     * @brief This class manages file input and output operations.
     *
     * The FileHandler class encapsulates the opening and closing of input and output files, ensuring
     * that files are properly managed and closed when no longer needed. It provides a simple interface
     * for file operations within an RAII context.
     */
    class FileHandler {
    public:
        std::ifstream inputFile; /**< Input file stream for reading data. */
        std::ofstream outputFile; /**< Output file stream for writing data. */

        /**
         * @brief Constructs a new FileHandler object.
         *
         * Opens the specified input and output files. Throws an exception if the files cannot be opened.
         *
         * @param inputFilename The path to the input file.
         * @param outputFilename The path to the output file.
         */
        FileHandler(const std::string &inputFilename, const std::string &outputFilename);

        /**
         * @brief Destroys the FileHandler object.
         *
         * Ensures that the input and output files are properly closed.
         */
        ~FileHandler();
    };
} // namespace file

#endif // FILEHANDLER_H
