# This script should be placed in Team24 directory.
import os
import glob

source_query_test_cases = [
    ("01_1_follows_followsT_source", "01_1_follows_followsT_queries"), ("02_1_follows_followsT_source", "02_1_follows_followsT_t_queries"), ("03_1_follows_test_source", "03_1_follows_test_queries"), ("04_1_followsT_test_source", "04_1_followsT_test_queries"), ("05_1_mixed_first_source",
                                                                                                                                                                                                                                                                     "05_1_mixed_first_queries"), ("06_1_mixed_invalid_source", "06_1_mixed_queries"), ("07_1_mixed_second_source", "07_1_mixed_second_queries"), ("08_1_modifies_test_source", "08_1_modifies_test_queries"), ("09_1_mixed_first_source", "09_1_parent_only_first_queries"),
    ("10_1_mixed_first_source", "10_1_uses_only_first_queries"), ("11_1_test_1a_source", "11_1_test_1a_queries"), ("12_1_test_1b_source", "12_1_test_1b_queries"), ("13_1_test_1c_source", "13_1_test_1c_queries"), ("14_1_test_1d_source", "14_1_test_1d_queries"), (
        "15_1_test_1t_source", "15_1_test_1t_queries"), ("16_1_test_2a_source", "16_1_test_2a_queries"), ("17_1_test_2b_source", "17_1_test_2b_queries"), ("18_1_test_3a_source", "18_1_test_3a_queries"), ("19_2_test_1_source", "19_2_test_1_uses_tup_boolean_queries"),
    ("20_2_test_1_attr_source", "20_2_test_1_attr_queries"), (
        "21_2_withcl_calidation_1a_test_source", "21_2_withcl_validation_1a_queries"),
    ("22_2_calls_callsT_source", "22_2_calls_callsT_queries"),
    ("23_2_pattern_whileIf_source", "23_2_pattern_whileIf_queries"),
    ("24_2_withCl_test_1_source", "24_2_withCl_test_1a_queries"),
    ("25_2_test_1_source", "25_2_test_1_TEST_queries"),
    ("26_2_system_test_1_source", "26_2_system_test_1_queries"),
    ("27_2_system_test_1_source", "27_2_system_test_1_test_queries"),
    ("28_2_system_test_2_source", "28_2_system_test_2_queries"),
    ("29_2_demo_1_source", "29_2_demo_1_queries"),
    ("30_2_demo_2_source", "30_2_demo_2_queries"),
    ("31_2_demo_source", "31_2_demo_queries"), ("32_3_source", "32_3_affectsBip_BipT_queries"), ("33_3_source", "33_3_nextBip_BipT_queries")]


autotester_dir = ".\\Code24\\Release\\"
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
        # os.system("{}autotester.exe {}{}.txt {}{}.txt {}{}_output.xml".format(autotester_dir,
        # test_cases_dir, source_file,
        # test_cases_dir, query_file,
        # output_dir, query_file))
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

    sorted(output_files, key=lambda x: int(x[18:20]))

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


check_dirs()
run_tests()
analyse_results()
