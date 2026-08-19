import sys
import pathlib
import numpy as np
import pytest

sys.path.append(str(pathlib.Path(__file__).resolve().parents[1]))
import validators

def test_max_abs_diff():
    a = np.array([0.0, 1.0, 2.0])
    b = np.array([0.0, 1.1, 1.9])
    assert validators.max_abs_diff(a, b) == pytest.approx(0.1, rel=1e-6)
