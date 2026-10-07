#!/usr/bin/env python3

from __future__ import annotations

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
BUNDLER = ROOT / "tools" / "bundle.py"
FIXTURE = ROOT / "tests" / "fixtures" / "solution.cpp"


class BundleTests(unittest.TestCase):
    def run_bundle(self, source: Path, *arguments: str) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            [sys.executable, str(BUNDLER), str(source), *arguments],
            cwd=ROOT,
            text=True,
            capture_output=True,
            check=False,
        )

    def test_selects_used_modules_and_produces_standalone_source(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "submission.cpp"
            executable = Path(directory) / "submission"
            result = self.run_bundle(FIXTURE, "-o", str(output), "--explain")
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("selected types, dsu, primality", result.stderr)

            bundled = output.read_text(encoding="utf-8")
            self.assertIn("class DSU", bundled)
            self.assertIn("inline bool is_prime(T n)", bundled)
            self.assertIn("using u32 =", bundled)
            self.assertNotIn("class RMQ", bundled)
            self.assertNotIn("class Fenwick", bundled)
            self.assertNotIn("#include <milmon/all.hpp>", bundled)
            self.assertLess(
                bundled.index("// milmon-lib/types.hpp"),
                bundled.index("// milmon-lib/ds/dsu.hpp"),
            )
            self.assertLess(
                bundled.index("// milmon-lib/ds/dsu.hpp"),
                bundled.index("// milmon-lib/math/primality.hpp"),
            )
            self.assertEqual(bundled.count("// milmon-lib ends"), 1)

            compile_result = subprocess.run(
                ["g++", "-std=c++17", "-O2", str(output), "-o", str(executable)],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
            run_result = subprocess.run(
                [str(executable)], text=True, capture_output=True, check=False
            )
            self.assertEqual(run_result.returncode, 0, run_result.stderr)
            self.assertEqual(run_result.stdout, "1\n")

    def test_source_directive_forces_a_module(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            source.write_text(
                "#include <milmon/all.hpp>\n"
                "// milmon: require rmq\n"
                "int main() {}\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("class RMQ", result.stdout)
            self.assertNotIn("class DSU", result.stdout)

    def test_direct_module_include_is_expanded(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            source.write_text(
                "#include <milmon/ds/dsu.hpp>\n"
                "int main() { cp::DSU d(1); return d.size(0) - 1; }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("class DSU", result.stdout)

    def test_debug_macro_is_selected_and_compiles(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            submission = Path(directory) / "submission.cpp"
            executable = Path(directory) / "submission"
            source.write_text(
                "#include <string>\n"
                "#include <vector>\n"
                "#include <milmon/all.hpp>\n"
                "int main() { debug(\"raw = %d\\n\", 5); int value = 7; "
                "std::vector<int> values{1, 2}; "
                "dbg(value,values,std::vector<int>{3, 4},\"a,b\"); }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source, "-o", str(submission), "--explain")
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("selected debug", result.stderr)
            bundled = submission.read_text(encoding="utf-8")
            self.assertIn("#define debug(...) std::fprintf(stderr, __VA_ARGS__)", bundled)
            self.assertIn("#define dbg(...) debug_impl(#__VA_ARGS__, __VA_ARGS__)", bundled)
            self.assertNotIn("class DSU", bundled)
            compile_result = subprocess.run(
                ["g++", "-std=c++17", "-O2", str(submission), "-o", str(executable)],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
            run_result = subprocess.run(
                [str(executable)], text=True, capture_output=True, check=False
            )
            self.assertEqual(run_result.returncode, 0, run_result.stderr)
            self.assertEqual(
                run_result.stderr,
                'raw = 5\nvalue=7, values=[1,2], '
                'std::vector<int>{3, 4}=[3,4], '
                '"a,b"=a,b\n',
            )

    def test_basic_io_is_selected_and_compiles(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            submission = Path(directory) / "submission.cpp"
            executable = Path(directory) / "submission"
            source.write_text(
                "#include <milmon/all.hpp>\n"
                "int main() { cp::init_io(3); std::cout << 1.25 << endl; }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source, "-o", str(submission), "--explain")
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("selected basic", result.stderr)
            bundled = submission.read_text(encoding="utf-8")
            self.assertIn("#define endl '\\n'", bundled)
            self.assertNotIn("class DSU", bundled)
            compile_result = subprocess.run(
                ["g++", "-std=c++17", "-O2", str(submission), "-o", str(executable)],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
            run_result = subprocess.run(
                [str(executable)], text=True, capture_output=True, check=False
            )
            self.assertEqual(run_result.returncode, 0, run_result.stderr)
            self.assertEqual(run_result.stdout, "1.250\n")

    def test_file_io_redirects_standard_streams(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            temporary = Path(directory)
            source = temporary / "main.cpp"
            submission = temporary / "submission.cpp"
            executable = temporary / "submission"
            source.write_text(
                "#include <milmon/all.hpp>\n"
                "int main() { cp::file_io(\"sample\"); int x; std::cin >> x; "
                "std::cout << x * 2 << '\\n'; }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source, "-o", str(submission), "--explain")
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("selected basic", result.stderr)
            compile_result = subprocess.run(
                ["g++", "-std=c++17", "-O2", str(submission), "-o", str(executable)],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
            (temporary / "sample.in").write_text("21\n", encoding="utf-8")
            run_result = subprocess.run(
                [str(executable)], cwd=temporary, text=True, capture_output=True, check=False
            )
            self.assertEqual(run_result.returncode, 0, run_result.stderr)
            self.assertEqual(run_result.stdout, "")
            self.assertEqual(
                (temporary / "sample.out").read_text(encoding="utf-8"), "42\n"
            )

    def test_pollard_rho_pulls_transitive_dependencies(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            submission = Path(directory) / "submission.cpp"
            executable = Path(directory) / "submission"
            source.write_text(
                "#include <milmon/all.hpp>\n"
                "int main() { auto f = cp::factorize_pair(u64{91}); "
                "return f.size() != 2 || f[0] != std::pair<u64, int>{7, 1} "
                "|| f[1] != std::pair<u64, int>{13, 1}; }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source, "-o", str(submission), "--explain")
            self.assertEqual(result.returncode, 0, result.stderr)
            compile_result = subprocess.run(
                ["g++", "-std=c++17", "-O2", str(submission), "-o", str(executable)],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
            run_result = subprocess.run(
                [str(executable)], text=True, capture_output=True, check=False
            )
            self.assertEqual(run_result.returncode, 0, run_result.stderr)

    def test_math_helpers_are_selected_and_compile(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            submission = Path(directory) / "submission.cpp"
            executable = Path(directory) / "submission"
            source.write_text(
                "#include <milmon/all.hpp>\n"
                "int main() { long long x, y; auto g = cp::exgcd(30LL, 18LL, x, y); "
                "auto s = cp::floor_sum(4, 10, 6, 3); "
                "auto [s0, s1, s2] = cp::ex_floor_sum(4, 10, 6, 3); "
                "return g != 6 || 30 * x + 18 * y != g || s != 3 || "
                "s0 != 3 || s1 != 8 || s2 != 5; }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source, "-o", str(submission), "--explain")
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("selected types, exgcd, ex_floor_sum, floor_sum", result.stderr)
            bundled = submission.read_text(encoding="utf-8")
            self.assertIn("inline T exgcd", bundled)
            self.assertIn("inline std::tuple<i128, i128, i128> ex_floor_sum", bundled)
            self.assertIn("inline i64 floor_sum", bundled)
            compile_result = subprocess.run(
                ["g++", "-std=c++17", "-O2", str(submission), "-o", str(executable)],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
            run_result = subprocess.run(
                [str(executable)], text=True, capture_output=True, check=False
            )
            self.assertEqual(run_result.returncode, 0, run_result.stderr)

    def test_frac_is_selected_and_compiles(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            submission = Path(directory) / "submission.cpp"
            executable = Path(directory) / "submission"
            source.write_text(
                "#include <milmon/all.hpp>\n"
                "int main() { i128 n = i128(1) << 100; "
                "cp::frac<i128> a{n, 3}, b{n, 6}; "
                "return a + b != cp::frac<i128>{n / 2}; }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source, "-o", str(submission), "--explain")
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("selected types, frac", result.stderr)
            bundled = submission.read_text(encoding="utf-8")
            self.assertIn("struct frac", bundled)
            self.assertNotIn("inline i64 floor_sum", bundled)
            compile_result = subprocess.run(
                ["g++", "-std=c++17", "-O2", str(submission), "-o", str(executable)],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
            run_result = subprocess.run(
                [str(executable)], text=True, capture_output=True, check=False
            )
            self.assertEqual(run_result.returncode, 0, run_result.stderr)

    def test_poly_mul_is_selected_and_compiles(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            submission = Path(directory) / "submission.cpp"
            executable = Path(directory) / "submission"
            source.write_text(
                "#include <milmon/all.hpp>\n"
                "int main() { cp::poly::init(); "
                "auto c = cp::poly::poly_mul({1, 2, 3}, {4, 5, 6}); "
                "return c != std::vector<u32>{4, 13, 28, 27, 18}; }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source, "-o", str(submission), "--explain")
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("selected types, poly_mul", result.stderr)
            bundled = submission.read_text(encoding="utf-8")
            self.assertIn("inline std::vector<u32> poly_mul", bundled)
            compile_result = subprocess.run(
                ["g++", "-std=c++17", "-O2", str(submission), "-o", str(executable)],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
            run_result = subprocess.run(
                [str(executable)], text=True, capture_output=True, check=False
            )
            self.assertEqual(run_result.returncode, 0, run_result.stderr)

    def test_pow_is_selected_and_compiles(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            submission = Path(directory) / "submission.cpp"
            executable = Path(directory) / "submission"
            source.write_text(
                "#include <milmon/all.hpp>\n"
                "int main() { return cp::pow<1000000007>(2, 10) != 1024 || "
                "cp::inv<1000000007>(2) != 500000004; }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source, "-o", str(submission), "--explain")
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("selected types, pow", result.stderr)
            bundled = submission.read_text(encoding="utf-8")
            self.assertIn("inline u32 pow", bundled)
            self.assertIn("inline u32 inv", bundled)
            compile_result = subprocess.run(
                ["g++", "-std=c++17", "-O2", str(submission), "-o", str(executable)],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
            run_result = subprocess.run(
                [str(executable)], text=True, capture_output=True, check=False
            )
            self.assertEqual(run_result.returncode, 0, run_result.stderr)

    def test_pair_alias_compiles_without_other_type_symbols(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            submission = Path(directory) / "submission.cpp"
            executable = Path(directory) / "submission"
            source.write_text(
                "#include <iostream>\n"
                "#include <milmon/all.hpp>\n"
                "int main() { pii value{3, 7}; "
                "std::cout << value.first << ' ' << value.second << '\\n'; }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source, "-o", str(submission))
            self.assertEqual(result.returncode, 0, result.stderr)
            compile_result = subprocess.run(
                ["g++", "-std=c++17", "-O2", str(submission), "-o", str(executable)],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
            run_result = subprocess.run(
                [str(executable)], text=True, capture_output=True, check=False
            )
            self.assertEqual(run_result.returncode, 0, run_result.stderr)
            self.assertEqual(run_result.stdout, "3 7\n")

    def test_fenwick_and_global_alias_are_selected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            source.write_text(
                "#include <milmon/all.hpp>\n"
                "int main() { cp::Fenwick<i64> f(3); f.add(1, 5); "
                "return f.prefix_sum(2) - 5; }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("using u32 =", result.stdout)
            self.assertIn("class Fenwick", result.stdout)
            self.assertNotIn("class DSU", result.stdout)

    def test_max_flow_is_selected_and_compiles(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            submission = Path(directory) / "submission.cpp"
            executable = Path(directory) / "submission"
            source.write_text(
                "#include <milmon/all.hpp>\n"
                "int main() { cp::MaxFlow<int> mf(2, 1); mf.add_edge(0, 1, 3); "
                "return mf.flow(0, 1) != 3; }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source, "-o", str(submission), "--explain")
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("selected max_flow", result.stderr)
            bundled = submission.read_text(encoding="utf-8")
            self.assertIn("class MaxFlow", bundled)
            compile_result = subprocess.run(
                ["g++", "-std=c++17", "-O2", str(submission), "-o", str(executable)],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
            run_result = subprocess.run(
                [str(executable)], text=True, capture_output=True, check=False
            )
            self.assertEqual(run_result.returncode, 0, run_result.stderr)

    def test_min_cost_flow_is_selected_and_compiles(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            submission = Path(directory) / "submission.cpp"
            executable = Path(directory) / "submission"
            source.write_text(
                "#include <milmon/all.hpp>\n"
                "int main() { cp::MinCostFlow<int, long long> mf(2, 1); "
                "mf.add_edge(0, 1, 3, -2); "
                "return mf.flow(0, 1) != std::pair<int, long long>{3, -6}; }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source, "-o", str(submission), "--explain")
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("selected min_cost_flow", result.stderr)
            bundled = submission.read_text(encoding="utf-8")
            self.assertIn("class MinCostFlow", bundled)
            self.assertNotIn("class MaxFlow", bundled)
            compile_result = subprocess.run(
                ["g++", "-std=c++17", "-O2", str(submission), "-o", str(executable)],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
            run_result = subprocess.run(
                [str(executable)], text=True, capture_output=True, check=False
            )
            self.assertEqual(run_result.returncode, 0, run_result.stderr)

    def test_rmq_symbol_selects_only_rmq(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            submission = Path(directory) / "submission.cpp"
            executable = Path(directory) / "submission"
            source.write_text(
                "#include <vector>\n"
                "#include <milmon/all.hpp>\n"
                "int main() { cp::RMQ<int> q(std::vector<int>{3, 1, 2}); "
                "return q.query(0, 3) - 1; }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("class RMQ", result.stdout)
            self.assertNotIn("class DSU", result.stdout)
            submission.write_text(result.stdout, encoding="utf-8")
            compile_result = subprocess.run(
                ["g++", "-std=c++17", "-O2", str(submission), "-o", str(executable)],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
            run_result = subprocess.run(
                [str(executable)], text=True, capture_output=True, check=False
            )
            self.assertEqual(run_result.returncode, 0, run_result.stderr)

    def test_convex_hull_pulls_point_type_and_compiles(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            submission = Path(directory) / "submission.cpp"
            executable = Path(directory) / "submission"
            source.write_text(
                "#include <milmon/all.hpp>\n"
                "int main() { auto h = cp::convex_hull({{0, 0}, {2, 0}, "
                "{1, 1}, {1, 0}}); return h.size() != 3; }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source, "-o", str(submission), "--explain")
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("selected p2, convex_hull", result.stderr)
            bundled = submission.read_text(encoding="utf-8")
            self.assertLess(bundled.index("struct p2"), bundled.index("convex_hull"))
            compile_result = subprocess.run(
                ["g++", "-std=c++17", "-O2", str(submission), "-o", str(executable)],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
            run_result = subprocess.run(
                [str(executable)], text=True, capture_output=True, check=False
            )
            self.assertEqual(run_result.returncode, 0, run_result.stderr)

    def test_real_point_and_long_double_alias_are_bundled(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            submission = Path(directory) / "submission.cpp"
            executable = Path(directory) / "submission"
            source.write_text(
                "#include <milmon/all.hpp>\n"
                "int main() { cp::p2r<ld> a{1, 2}, b{3, 4}; "
                "return cp::cross(a, b) != -2; }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source, "-o", str(submission), "--explain")
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("selected types, p2r", result.stderr)
            bundled = submission.read_text(encoding="utf-8")
            self.assertIn("using ld = long double", bundled)
            self.assertIn("struct p2r", bundled)
            compile_result = subprocess.run(
                ["g++", "-std=c++17", "-O2", str(submission), "-o", str(executable)],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
            run_result = subprocess.run(
                [str(executable)], text=True, capture_output=True, check=False
            )
            self.assertEqual(run_result.returncode, 0, run_result.stderr)

    def test_suffix_array_pulls_rmq_and_compiles(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            submission = Path(directory) / "submission.cpp"
            executable = Path(directory) / "submission"
            source.write_text(
                "#include <vector>\n"
                "#include <string>\n"
                "#include <vector>\n"
                "#include <milmon/all.hpp>\n"
                "int main() { auto a = cp::build_sa(std::string(\"banana\")); "
                "cp::SA s(std::string(\"banana\")); return a[0] != 5 || "
                "s.sa[0] != 5 || s.rnk[0] != 3 || s.lcp(1, 3) != 3; }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source, "-o", str(submission), "--explain")
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("selected rmq, suffix_array", result.stderr)
            bundled = submission.read_text(encoding="utf-8")
            self.assertIn("class RMQ", bundled)
            self.assertIn("struct SuffixArray", bundled)
            self.assertEqual(bundled.count("#include <vector>"), 1)
            self.assertEqual(bundled.count("#include <string>"), 1)
            self.assertLess(
                bundled.rfind("#include <"), bundled.index("// milmon-lib/ds/rmq.hpp")
            )
            self.assertLess(
                bundled.index("// milmon-lib/ds/rmq.hpp"),
                bundled.index("// milmon-lib/string/suffix_array.hpp"),
            )
            self.assertGreater(
                bundled.index("// milmon-lib ends"),
                bundled.index("// milmon-lib/string/suffix_array.hpp"),
            )
            compile_result = subprocess.run(
                ["g++", "-std=c++17", "-O2", str(submission), "-o", str(executable)],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
            run_result = subprocess.run(
                [str(executable)], text=True, capture_output=True, check=False
            )
            self.assertEqual(run_result.returncode, 0, run_result.stderr)

    def test_empty_selection_adds_no_library_markers(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            source.write_text(
                "#include <vector>\n"
                "#include <milmon/all.hpp>\n"
                "int main() { std::vector<int> a; return a.size(); }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout.count("#include <vector>"), 1)
            self.assertNotIn("milmon-lib/", result.stdout)
            self.assertNotIn("milmon-lib ends", result.stdout)

    def test_string_algorithms_are_selected_and_compile(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            submission = Path(directory) / "submission.cpp"
            executable = Path(directory) / "submission"
            source.write_text(
                "#include <string>\n"
                "#include <milmon/all.hpp>\n"
                "int main() { std::string s = \"ababa\"; "
                "auto p = cp::kmp(s); auto z = cp::z_function(s); "
                "auto m = cp::manacher(s); auto l = cp::build_lyndon(s); "
                "return p[4] != 3 || z[2] != 3 || m[5] != 5 || l.empty(); }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source, "-o", str(submission), "--explain")
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("selected kmp, lyndon, manacher, z_function", result.stderr)
            bundled = submission.read_text(encoding="utf-8")
            self.assertNotIn("struct SuffixArray", bundled)
            self.assertNotIn("class RMQ", bundled)
            compile_result = subprocess.run(
                ["g++", "-std=c++17", "-O2", str(submission), "-o", str(executable)],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
            run_result = subprocess.run(
                [str(executable)], text=True, capture_output=True, check=False
            )
            self.assertEqual(run_result.returncode, 0, run_result.stderr)

    def test_dependency_closure_uses_manifest_order_safely(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            temporary = Path(directory)
            (temporary / "base.hpp").write_text(
                "#pragma once\ninline int base_value() { return 7; }\n", encoding="utf-8"
            )
            (temporary / "feature.hpp").write_text(
                '#pragma once\n#include "base.hpp"\n'
                "inline int feature_value() { return base_value(); }\n",
                encoding="utf-8",
            )
            manifest = {
                "version": 1,
                "umbrella_headers": ["demo/all.hpp"],
                "modules": {
                    "feature": {
                        "header": "feature.hpp",
                        "symbols": ["feature_value"],
                        "depends": ["base"],
                    },
                    "base": {
                        "header": "base.hpp",
                        "symbols": ["base_value"],
                        "depends": [],
                    },
                },
            }
            manifest_path = temporary / "library.json"
            manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
            source = temporary / "main.cpp"
            source.write_text(
                "#include <demo/all.hpp>\nint main() { return feature_value() - 7; }\n",
                encoding="utf-8",
            )
            result = self.run_bundle(source, "--manifest", str(manifest_path))
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertLess(
                result.stdout.index("inline int base_value()"),
                result.stdout.index("inline int feature_value()"),
            )

    def test_refuses_to_overwrite_input(self) -> None:
        result = self.run_bundle(FIXTURE, "-o", str(FIXTURE))
        self.assertEqual(result.returncode, 2)
        self.assertIn("refusing to overwrite", result.stderr)

    def test_source_without_library_include_is_unchanged(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.cpp"
            content = (
                "#include <vector>\n"
                "#include <vector>\n"
                "\n"
                "/*\n"
                "#include <milmon/all.hpp>\n"
                "*/\n"
                "// keep this layout\n"
                "int main() {\n"
                "    return 0;\n"
                "}\n"
            )
            source.write_text(content, encoding="utf-8")
            result = self.run_bundle(source)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout, content)


if __name__ == "__main__":
    unittest.main()
