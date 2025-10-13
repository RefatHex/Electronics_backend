import pandas as pd
import numpy as np
from pathlib import Path
from sklearn.preprocessing import LabelEncoder, StandardScaler
from sklearn.ensemble import RandomForestClassifier
from sklearn.multioutput import MultiOutputClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score
import joblib
from typing import Dict, Any

# Paths and artifacts
base_dir = Path(__file__).parent
data_path = base_dir / 'data.xlsx'
artifacts_path = base_dir / 'ml_artifacts.joblib'


def _load_dataframe(path: Path) -> pd.DataFrame:
    if not path.exists():
        raise FileNotFoundError(f"data.xlsx not found at {path}. Place data.xlsx next to ML.py or provide correct path")
    return pd.read_excel(path)


def train_and_save(data_path: Path = data_path, artifacts_file: Path = artifacts_path, epochs: int = 5, batch_trees: int = 20) -> Dict[str, Any]:
    """Train the multi-output classifier and save model+scaler+encoders to artifacts_file.

    Returns the artifacts dict.
    """
    df = _load_dataframe(data_path)

    # Split sensors and decision columns
    X = df[['temperature_C', 'pH', 'turbidity_NTU', 'flow_m_s', 'trash_detected']].copy()
    y = df[['water_quality', 'action_decision', 'pollution_alert',
            'flow_condition', 'cleaning_recommendation', 'safety_alert']].copy()

    # Convert trash_detected to numeric
    X['trash_detected'] = X['trash_detected'].map({'Yes': 1, 'No': 0})

    # Encode categorical outputs
    encoders = {}
    for col in y.columns:
        le = LabelEncoder()
        y[col] = le.fit_transform(y[col])
        encoders[col] = le

    # Train/test split (50%/50%)
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.5, random_state=42)
    scaler = StandardScaler()
    X_train_scaled = scaler.fit_transform(X_train)
    X_test_scaled = scaler.transform(X_test)

    # Simulate epochs & batches for status printing
    print("Training progress:")
    for epoch in range(1, epochs + 1):
        n_estimators = epoch * batch_trees
        temp_model = MultiOutputClassifier(RandomForestClassifier(n_estimators=n_estimators, random_state=42))
        temp_model.fit(X_train_scaled, y_train)
        print(f"\nEpoch {epoch}/{epochs} - Trees: {n_estimators}")
        for i, col in enumerate(y.columns):
            acc = accuracy_score(y_test.iloc[:, i], temp_model.predict(X_test_scaled)[:, i])
            print(f"  {col}: {acc * 100:.2f}%")

    # Final training on full data
    final_trees = epochs * batch_trees
    model = MultiOutputClassifier(RandomForestClassifier(n_estimators=final_trees, random_state=42))
    model.fit(X_train_scaled, y_train)

    print("\nFinal accuracy per output:")
    for i, col in enumerate(y.columns):
        acc = accuracy_score(y_test.iloc[:, i], model.predict(X_test_scaled)[:, i])
        print(f"{col}: {acc * 100:.2f}%")
    print("Model training complete.\n")

    # Save artifacts in one file
    artifacts = {
        'model': model,
        'scaler': scaler,
        'encoders': encoders,
        'columns': X.columns.tolist()
    }
    joblib.dump(artifacts, artifacts_file)
    print(f"Saved artifacts to: {artifacts_file}")
    return artifacts


# Module-level cache for loaded artifacts
_ARTIFACTS: Dict[str, Any] = {}


def load_artifacts(artifacts_file: Path = artifacts_path) -> Dict[str, Any]:
    """Load model artifacts into module cache and return them."""
    global _ARTIFACTS
    if _ARTIFACTS:
        return _ARTIFACTS
    if not artifacts_file.exists():
        raise FileNotFoundError(f"Artifacts file not found at {artifacts_file}. Run ML.py as script to train and save artifacts first.")
    _ARTIFACTS = joblib.load(artifacts_file)
    return _ARTIFACTS


def predict(features: Dict[str, Any]) -> Dict[str, Any]:
    """Predict outputs for a single sample represented as a features dict.

    Example features keys: 'temperature_C','pH','turbidity_NTU','flow_m_s','trash_detected'
    """
    artifacts = load_artifacts()
    model = artifacts['model']
    scaler = artifacts['scaler']
    encoders = artifacts['encoders']
    columns = artifacts['columns']

    # Normalize trash_detected input
    td = features.get('trash_detected')
    if isinstance(td, str):
        td = 1 if td.lower() in ('yes', '1', 'true') else 0
    elif td is None:
        td = 0

    row = [features.get(col, 0) if col != 'trash_detected' else td for col in columns]
    df_row = pd.DataFrame([row], columns=columns)
    X_scaled = scaler.transform(df_row)
    pred_nums = model.predict(X_scaled)[0]

    result = {}
    for i, col in enumerate(encoders.keys()):
        num = int(pred_nums[i])
        label = encoders[col].inverse_transform([num])[0]
        result[col] = {'label': label, 'numeric': num}
    return result


def predict_from_latest_excel(path: Path = data_path) -> Dict[str, Any]:
    df = _load_dataframe(path)
    X = df[['temperature_C', 'pH', 'turbidity_NTU', 'flow_m_s', 'trash_detected']].copy()
    latest = X.iloc[-1]
    features = latest.to_dict()
    return predict(features)


if __name__ == '__main__':
    # When run directly, train and save artifacts and print a sample prediction
    artifacts = train_and_save()
    print(f"Artifacts saved at: {artifacts_path}")
    # show a sample prediction from the latest row
    try:
        sample_pred = predict_from_latest_excel()
        print("Sample prediction from latest excel row:")
        for k, v in sample_pred.items():
            print(f"{k}: {v['label']} ({v['numeric']})")
    except Exception as e:
        print(f"Could not produce sample prediction: {e}")
