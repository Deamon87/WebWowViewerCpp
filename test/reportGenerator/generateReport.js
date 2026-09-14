const fs = require('fs');
const path = require('path');
const xml2js = require('xml2js');

// Function to read and parse XML file (promisified)
function loadXML(filePath) {
    return new Promise((resolve, reject) => {
        fs.readFile(filePath, 'utf8', (err, data) => {
            if (err) {
                return reject(err);
            }

            const parser = new xml2js.Parser();
            parser.parseString(data, (parseErr, result) => {
                if (parseErr) {
                    return reject(parseErr);
                }
                resolve(result);
            });
        });
    });
}

// Function to find all *_report.xml files in first-level subdirectories
function findReportFiles(baseDir) {
    const reportFiles = [];

    // Scan first-level subdirectories
    try {
        const entries = fs.readdirSync(baseDir, { withFileTypes: true });

        for (const entry of entries) {
            if (entry.isDirectory()) {
                const subDir = path.join(baseDir, entry.name);
                try {
                    const files = fs.readdirSync(subDir);
                    for (const file of files) {
                        if (file.endsWith('_report.xml')) {
                            reportFiles.push(path.join(subDir, file));
                        }
                    }
                } catch (err) {
                    console.error(`Error reading directory ${subDir}:`, err.message);
                }
            }
        }
    } catch (err) {
        console.error(`Error scanning directories in ${baseDir}:`, err.message);
    }

    return reportFiles;
}

// Function to group test cases by name and extract image paths
function groupTestCasesByName(xmlData) {
    const testCaseGroups = {};

    if (!xmlData.testsuites || !xmlData.testsuites.testsuite) {
        return testCaseGroups;
    }

    const testSuites = xmlData.testsuites.testsuite;

    testSuites.forEach(testsuite => {
        // Extract the "NameInQuestion" part from the test suite name
        const suiteNameParts = testsuite.$.name.split('/');
        const suiteName = suiteNameParts[0];

        if (testsuite.testcase) {
            testsuite.testcase.forEach(testcase => {
                let pngFilePath = null;

                if (testcase.properties) {
                    testcase.properties.forEach(property => {
                        if (property.property) {
                            property.property.forEach(prop => {
                                if (prop.$.name === 'pngFile') {
                                    pngFilePath = prop.$.value;
                                }
                            });
                        }
                    });
                }

                const caseName = testcase.$.name.split('/')[0]; // Extract the base name

                if (!testCaseGroups[caseName]) {
                    testCaseGroups[caseName] = {};
                }

                if (!testCaseGroups[caseName][suiteName]) {
                    testCaseGroups[caseName][suiteName] = [];
                }

                if (pngFilePath) {
                    testCaseGroups[caseName][suiteName].push({
                        path: pngFilePath,
                        fullName: testcase.$.name
                    });
                }
            });
        }
    });

    return testCaseGroups;
}

// Function to merge test case groups from multiple XML files
function mergeTestCaseGroups(allGroups) {
    const merged = {};

    for (const groups of allGroups) {
        for (const [caseName, cases] of Object.entries(groups)) {
            if (!merged[caseName]) {
                merged[caseName] = {};
            }

            for (const [suiteName, images] of Object.entries(cases)) {
                if (!merged[caseName][suiteName]) {
                    merged[caseName][suiteName] = [];
                }
                merged[caseName][suiteName].push(...images);
            }
        }
    }

    return merged;
}

// Function to generate HTML content
function generateHTML(imageGroups, baseDir) {
    let htmlContent = `
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Test Case Images</title>
    <style>
        .image-group {
            display: flex;
            flex-wrap: nowrap;
            margin-top: 32px;
        }
        .image-group img {
            max-height: 768px;
            margin-right: 10px;
            margin-bottom: 10px;
        }
        .testcase-caption {
            font-weight: bold;
            margin-top: 10px;
            font-size: 15px;
            display: block;

            font-family: 'Open Sans', sans-serif;

            position: absolute;
            top: -34px;
            letter-spacing: 1.5px;
        }
        .container {
            position: relative;
        }
    </style>
</head>
<body>
    <h1>Test Case Images</h1>
    <div id="image-groups">
`;

    for (const [caseName, cases] of Object.entries(imageGroups)) {
        htmlContent += `<h2>${caseName}</h2>`;

        htmlContent += `<div class="image-group">`;
        for (const [suiteName, images] of Object.entries(cases)) {
            images.forEach(image => {
                const fullName = image.fullName;
                // Resolve path relative to baseDir
                const imagePath = path.isAbsolute(image.path)
                    ? image.path
                    : path.resolve(baseDir, image.path);
                htmlContent += `
                <div class="container">
                    <img src="file://${imagePath}" alt="${fullName} image" title="${fullName}">
                    <div class="testcase-caption"> - ${suiteName}</div>
                </div>`;
            });
        }
        htmlContent += '</div>';
    }

    htmlContent += `
    </div>
</body>
</html>
`;

    return htmlContent;
}

// Main function to load XML files, process data and generate HTML
async function main() {
    // Get folder path from command line
    const args = process.argv.slice(2);

    if (args.length === 0) {
        console.error('Usage: node generateReport.js <test-results-folder>');
        console.error('Example: node generateReport.js ../cmake-build-release/testResults');
        process.exit(1);
    }

    const baseDir = path.resolve(args[0]);

    if (!fs.existsSync(baseDir)) {
        console.error(`Error: Directory does not exist: ${baseDir}`);
        process.exit(1);
    }

    console.log(`Scanning for report files in: ${baseDir}`);

    // Find all *_report.xml files
    const reportFiles = findReportFiles(baseDir);

    if (reportFiles.length === 0) {
        console.error('No *_report.xml files found in the specified directory or its subdirectories.');
        process.exit(1);
    }

    console.log(`Found ${reportFiles.length} report file(s):`);
    reportFiles.forEach(f => console.log(`  - ${f}`));

    // Load and parse all XML files
    const allGroups = [];

    for (const filePath of reportFiles) {
        try {
            console.log(`Loading: ${filePath}`);
            const xmlData = await loadXML(filePath);
            const groups = groupTestCasesByName(xmlData);
            allGroups.push(groups);
        } catch (err) {
            console.error(`Error loading ${filePath}:`, err.message);
        }
    }

    if (allGroups.length === 0) {
        console.error('No valid test data found in any report files.');
        process.exit(1);
    }

    // Merge all test case groups
    const mergedGroups = mergeTestCaseGroups(allGroups);

    // Generate HTML
    const htmlContent = generateHTML(mergedGroups, baseDir);

    // Write output HTML
    const outputHtmlPath = path.join(baseDir, 'report.html');
    fs.writeFileSync(outputHtmlPath, htmlContent, 'utf8');
    console.log(`HTML report generated successfully at ${outputHtmlPath}`);
}

main().catch(err => {
    console.error('Unexpected error:', err);
    process.exit(1);
});