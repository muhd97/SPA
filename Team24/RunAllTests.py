# This script should be placed in Team24 directory.
import os
import glob

source_query_test_cases = [("1.1_follows_followsT_source", "1.1_follows_followsT_queries"),
                           ("1.2_follows_followsT_source",
                            "1.2_follows_followsT_t_queries"),
                           ("1.3_follows_test_source", "1.3_follows_test_queries"),
                           ("1.4_followsT_test_source",
                            "1.4_followsT_test_queries"),
                           ("1.5_mixed_first_source", "1.5_mixed_first_queries"),
                           ("1.6_mixed_invalid_source", "1.6_mixed_queries"),
                           ("1.7_mixed_second_source", "1.7_mixed_second_queries"),
                           ("1.8_modifies_test_source",
                            "1.8_modifies_test_queries"),
                           ("1.9_mixed_first_source",
                            "1.9_parent_only_first_queries"),
                           ("1.10_mixed_first_source",
                            "1.10_uses_only_first_queries"),
                           ("1.11_test_1a_source", "1.11_test_1a_queries"),
                           ("1.12_test_1b_source", "1.12_test_1b_queries"),
                           ("1.13_test_1c_source", "1.13_test_1c_queries"),
                           ("1.14_test_1d_source", "1.14_test_1d_queries"),
                           ("1.15_test_1t_source", "1.15_test_1t_queries"),
                           ("1.16_test_2a_source", "1.16_test_2a_queries"),
                           ("1.17_test_2b_source", "1.17_test_2b_queries"),
                           ("1.18_test_3a_source", "1.18_test_3a_queries"),
                           ("2.1_test_1_source", "2.1_test_1_uses_tup_boolean_queries")]


autotester_dir = ".\\Code24\\Debug\\"
test_cases_dir = ".\\Tests24\\"
output_dir = ".\\Tests24\\Outputs\\"


def check_dirs():
    print("... Checking files and directories ...")
    print()
    # Build check
    if not os.path.exists(("{}AutoTester.exe".format(autotester_dir))):
        print("[X] AutoTester.exe not found. Please build the project first")
        exit(1)

    # Output folder check
    if not os.path.isdir(("{}".format(output_dir))):
        os.mkdir(output_dir)
        print()

    # Test cases (source and query) checks
    for t in source_query_test_cases:
        source_file = t[0]
        query_file = t[1]
        if not os.path.exists("{}{}.txt".format(test_cases_dir, source_file)):
            print("[X] {}.txt does not exist.".format(source_file))
            exit(1)
        if not os.path.exists("{}{}.txt".format(test_cases_dir, query_file)):
            print("[X] {}.txt does not exist.".format(query_file))
            exit(1)
    print("Check completed!")
    print()


def run_tests():
    for t in source_query_test_cases:
        source_file = t[0]
        query_file = t[1]
        os.popen("{}AutoTester.exe {}{}.txt {}{}.txt {}{}_output.xml".format(autotester_dir,
                                                                             test_cases_dir, source_file,
                                                                             test_cases_dir, query_file,
                                                                             output_dir, query_file)).read()


def analyse_results():
    print("... Analysing output xml files ...")
    print()
    output_files = sorted(glob.glob(output_dir + "*"))

    if ".\\Tests24\\Outputs\\analysis.xsl" in output_files:
        output_files.remove(".\\Tests24\\Outputs\\analysis.xsl")
        
    print(output_files)

    summary_of_all_test_cases = {}
    c = 0
    for xml in output_files:
        print("Test case # {}".format(c+1))
        result = {"total queries": 0, "total passed": 0,
                  "total failed": 0, "total timeouts": 0, "total exceptions": 0, "total crashes": 0}

        with open(xml) as f:
            for line in f:
                if '''/failed''' in line:
                    result["total failed"] += 1
                    print("[X] failed found in {}".format(xml))
                if '''/crash''' in line:
                    result["total crashes"] += 1
                    result["total failed"] += 1
                    print("[X] crash found in {}".format(xml))
                if '''passed''' in line:
                    result["total passed"] += 1
                if '''/exception''' in line:
                    result["exception"] += 1
                    result["total failed"] += 1
                    print("[X] exception found in {}".format(xml))
                if '''timeout''' in line:
                    result["total timeouts"] += 1
                    result["total failed"] += 1
                    print("[X] timeout found in {}".format(xml))
        result["total queries"] = result["total failed"] + \
            result["total passed"]

        if result["total failed"] == 0:
            print("Passed all queries")

        summary_of_all_test_cases[c] = result
        c += 1
        print()

    print("Finished analysing!")
    print()
    print("---Summary of all test cases---")
    print()
    print("*******************************************************************************************************")
    print("| Test case |   # total queries |   # passed |   # failed |   # exception |   # timeout |   # crashes |")
    print("*******************************************************************************************************")

    for r in summary_of_all_test_cases:
        print(f" {r+1:10} |", end="")
        print(f" {summary_of_all_test_cases[r]['total queries']:17} |", end="")
        print(f" {summary_of_all_test_cases[r]['total passed']:10} |", end="")
        print(f" {summary_of_all_test_cases[r]['total failed']:10} |", end="")
        print(
            f" {summary_of_all_test_cases[r]['total exceptions']:13} |", end="")
        print(
            f" {summary_of_all_test_cases[r]['total timeouts']:11} |", end="")
        print(f" {summary_of_all_test_cases[r]['total crashes']:11} |", end="")
        print()

    print("*******************************************************************************************************")


# check_dirs()
# run_tests()
analyse_results()
