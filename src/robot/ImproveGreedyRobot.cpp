#include "ImproveGreedyRobot.h"

namespace dab::__detail__::robot {

// True when some empty edge is "safe": drawing it leaves every neighbouring box with at most
// one undrawn edge, so it cannot hand the opponent a box. Safe moves exist only during the
// opening/midgame; their absence marks the start of the chains-and-loops endgame.
template <typename Board>
bool ImproveGreedyRobot::HasSafeMove(const Board& board) const {
  for (const model::Edge edge : board.EmptyEdges()) {
    if (board.MaxEdgeCount(edge) < 2) {
      return true;
    }
  }
  return false;
}

// Returns a scoreable edge that completes exactly one box. While taking a chain we prefer such
// edges so the chain shrinks one box at a time and the double-cross check fires once exactly two
// boxes remain.
template <typename Board>
model::Edge ImproveGreedyRobot::FindSingleBoxEdge(const Board& board) const {
  for (const model::Edge edge : board.EmptyEdges()) {
    if (board.MaxEdgeCount(edge) != 3) {
      continue;
    }
    Int near3 = 0;
    for (const model::Box box : edge.NearBoxes()) {
      if (board.EdgeCount(box) == 3) {
        ++near3;
      }
    }
    if (near3 == 1) {
      return edge;
    }
  }
  return model::Edge::Invalid;
}

// Counts how many boxes the side to move can score in a row right now (the current chain "run").
// Leaves SimulationBoardBackup in the fully-taken state, which the caller uses to test whether the
// game is over after the take.
template <typename Board>
Int ImproveGreedyRobot::TakableCount(const Board& board) {
  SimulationBoardBackup = board;
  return SimulationBoardBackup.MaxObtainableScore(model::Box::Max + 1);
}

// Opens the shortest remaining chain: among the ==2 edges (each hands the opponent a box), pick the
// one that lets the opponent take the fewest boxes. This is Berlekamp's "standard move" for the
// chains-and-loops endgame.
template <typename Board>
model::Edge ImproveGreedyRobot::OpenShortestChain(const Board& board) {
  model::Edge result = model::Edge::Invalid;
  Int minScore = model::Box::Max + 1;
  SimulationBoardBackup = board;
  for (const model::Edge edge : board.EmptyEdges()) {
    if (board.MaxEdgeCount(edge) != 2) {
      continue;
    }
    SimulationBoard = SimulationBoardBackup;
    SimulationBoard.Add(edge);
    if (const Int score = SimulationBoard.MaxObtainableScore(minScore); score < minScore) {
      minScore = score;
      result = edge;
    }
  }
  return result;
}

template <typename Board>
iterable::Span<const model::Edge> ImproveGreedyRobot::BestCandidateEdges(const Board& board) {
  if (const iterable::Span<const model::Edge> edges = GreedyRobot::BestCandidateEdges(board);
      EnemyUnscoreable() || Scoreable()) {
    return edges;
  }

  Int minScore = model::Box::Max + 1;

  CandidateEdgesSize = 0;
  SimulationBoardBackup = board;
  for (const model::Edge edge : board.EmptyEdges()) {
    SimulationBoard = SimulationBoardBackup;
    SimulationBoard.Add(edge);
    if (const Int score = SimulationBoard.MaxObtainableScore(minScore); score < minScore) {
      minScore = score;
      CandidateEdgesSize = 1;
      Edges.At(0) = edge;
    } else if (score == minScore) {
      Edges.At(CandidateEdgesSize++) = edge;
    }
  }

  return {Edges.begin(), CandidateEdgesSize};
}

template <typename Board>
model::Edge ImproveGreedyRobot::SearchOne(const Board& board) {
  model::Edge result = GreedyRobot::SearchOne(board);
  if (result == model::Edge::Invalid) {
    // No scoreable and no safe move: every empty edge hands the opponent a box. Open the shortest
    // chain.
    if (const model::Edge open = OpenShortestChain(board); open != model::Edge::Invalid) {
      return open;
    }
    return result;
  }
  if (board.MaxEdgeCount(result) <= 1) {
    return result;  // safe move; nothing to sacrifice yet
  }
  if (HasSafeMove(board)) {
    return result;  // still opening: a free box is always correct
  }

  // Chains-and-loops endgame. Apply the double-cross: take all but two boxes of a chain, then hand
  // the last two over to force the opponent to open the next chain.
  const Int takable = TakableCount(board);
  if (takable <= 1) {
    return result;  // a lone box
  }
  if (takable == 2 && SimulationBoardBackup.Gaming()) {
    // Two boxes left with more chains after them: double-cross by opening the shortest remaining
    // chain instead of taking the two boxes.
    if (const model::Edge open = OpenShortestChain(board); open != model::Edge::Invalid) {
      return open;
    }
  }
  if (const model::Edge single = FindSingleBoxEdge(board); single != model::Edge::Invalid) {
    return single;
  }
  return result;
}

template iterable::Span<const model::Edge> ImproveGreedyRobot::BestCandidateEdges<board::RelativeScoreBoard>(
    const board::RelativeScoreBoard& board);
template iterable::Span<const model::Edge> ImproveGreedyRobot::BestCandidateEdges<board::GameBoard>(
    const board::GameBoard& board);

template model::Edge ImproveGreedyRobot::SearchOne<board::RelativeScoreBoard>(const board::RelativeScoreBoard& board);

}  // namespace dab::__detail__::robot
